#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>
#include <poll.h>              // <= для wait_readable()

#include "router.hpp"
#include "handlers/health.hpp"
#include "handlers/docs.hpp"
#include "handlers/auth/auth_register.hpp"
#include "handlers/auth/auth_login.hpp"
#include "handlers/auth/auth_device_login.hpp"
#include "handlers/auth/auth_forgot.hpp"
#include "handlers/auth/auth_reset.hpp"
#include "handlers/auth/auth_change_password.hpp"
#include "handlers/auth/auth_logout.hpp"
#include "middleware/cors.hpp"

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp = net::ip::tcp;

static int env_int(const char* n, int def){
    if (const char* v = std::getenv(n)) { try { return std::stoi(v); } catch (...) {} }
    return def;
}

static int READ_TIMEOUT_MS = env_int("READ_TIMEOUT_MS", 300);
static int IDLE_TIMEOUT_S  = env_int("IDLE_TIMEOUT_S",  3);
static int MAX_REQS        = env_int("MAX_REQS",        5);
static int MAX_BODY        = env_int("MAX_BODY",        1024);
static int MAX_HDR         = env_int("MAX_HDR",         8*1024);

// сигнатура сборки — видно в /_meta и в баннере при старте
static const char* BUILD_SIG = __DATE__ " " __TIME__;

static http::response<http::string_body> make_error(http::status st, std::string msg){
    http::response<http::string_body> res; res.result(st); res.version(11);
    res.set(http::field::content_type, "application/json; charset=utf-8");
    res.keep_alive(false);
    res.body() = std::string("{\"status\":\"error\",\"message\":\"") + msg + "\"}";
    middleware::apply_cors(res); res.prepare_payload(); return res;
}


http::response<http::string_body> dispatch(const http::request<http::string_body>& req){
    if (is_options(req)) { http::response<http::string_body> r; middleware::set_preflight(r); return r; }

    const auto path = path_only(std::string_view(req.target().data(), req.target().size()));
    if (path == "/health") return handlers::health::handle(req);
    if (path == "/openapi.yaml") return handlers::docs::openapi_yaml();
    if (path == "/docs") return handlers::docs::swagger_ui();

    // /_meta — отладка
    if (path == "/_meta"){
        http::response<http::string_body> r; r.version(11); r.result(http::status::ok);
        r.set(http::field::content_type, "application/json; charset=utf-8");
        r.body() = std::string("{\"build\":\"") + BUILD_SIG + "\","
                  + "\"read_ms\":" + std::to_string(READ_TIMEOUT_MS) + ","
                  + "\"idle_s\":"  + std::to_string(IDLE_TIMEOUT_S)  + ","
                  + "\"max_reqs\":"+ std::to_string(MAX_REQS) + "}";
        middleware::apply_cors(r); r.prepare_payload(); return r;
    }

    if (path == "/auth/register") return handlers::auth::auth_register(req);
    if (path == "/auth/login") return handlers::auth::auth_login(req);
    if (path == "/auth/device-login") return handlers::auth::auth_device_login(req);
    if (path == "/auth/forgot") return handlers::auth::auth_forgot(req);
    if (path == "/auth/reset") return handlers::auth::auth_reset(req);
    if (path == "/auth/change-password") return handlers::auth::auth_change_password(req);
    if (path == "/auth/logout") return handlers::auth::auth_logout(req);

    http::response<http::string_body> r; r.result(http::status::not_found); r.version(11);
    r.set(http::field::content_type, "application/json; charset=utf-8");
    r.body() = R"({"status":"error","message":"Not Found"})";
    middleware::apply_cors(r); r.prepare_payload(); return r;
}

// helper: ждём готовность к чтению через poll()
static bool wait_readable(boost::asio::ip::tcp::socket& s, int timeout_ms) {
    if (timeout_ms <= 0) return true;
    struct pollfd pfd{};
    pfd.fd = s.native_handle();
    pfd.events = POLLIN;
    int r = ::poll(&pfd, 1, timeout_ms);
    if (r <= 0) return false; // ошибка или таймаут — считаем «нет данных»
    return (pfd.revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL)) != 0;
}

void session(boost::asio::ip::tcp::socket socket){
    namespace beast = boost::beast;
    namespace http  = beast::http;
    using boost::asio::ip::tcp;

    beast::tcp_stream stream{std::move(socket)};
    beast::flat_buffer buffer;
    int handled = 0;

    for(;;){
        buffer.consume(buffer.size());

        // --- 1) ЧТЕНИЕ ЗАГОЛОВКОВ С ЖЁСТКИМ ДЕДЛАЙНОМ И ЛИМИТОМ ---
        // Для первого запроса используем READ_TIMEOUT_MS,
        // для последующих — idle: сперва ждём появления данных не дольше IDLE_TIMEOUT_S.
        if (handled > 0) {
            if (IDLE_TIMEOUT_S > 0 && !wait_readable(stream.socket(), IDLE_TIMEOUT_S * 1000)) {
                // тишина — закрываем keep-alive без второго ответа
                break;
            }
        }

        const int header_deadline_ms = (handled == 0 ? READ_TIMEOUT_MS : IDLE_TIMEOUT_S * 1000);
        const std::size_t header_limit = (MAX_HDR > 0 ? (std::size_t)MAX_HDR : (std::size_t)8192);

        std::string header_raw;
        header_raw.reserve(1024);

        auto t0 = std::chrono::steady_clock::now();
        auto elapsed_ms = [&]{ 
            return (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t0).count();
        };

        bool header_ok = false;
        beast::error_code ec;

        // ручной цикл чтения до "\r\n\r\n"
        while (true) {
            int remain = header_deadline_ms - elapsed_ms();
            if (header_deadline_ms > 0 && remain <= 0) {
                // таймаут заголовка
                beast::error_code ig;
                http::write(stream, make_error(http::status::request_timeout, "read timeout (header)"), ig);
                return;
            }
            if (!wait_readable(stream.socket(), header_deadline_ms > 0 ? remain : 0)) {
                // не дождались — это тоже таймаут заголовка
                beast::error_code ig;
                http::write(stream, make_error(http::status::request_timeout, "read timeout (header)"), ig);
                return;
            }

            char chunk[1024];
            std::size_t n = 0;
            try {
                n = stream.socket().read_some(boost::asio::buffer(chunk, sizeof(chunk)), ec);
            } catch (...) {
                ec = beast::error_code{}; // приведём к пустому — ниже обработаем
            }
            if (ec == boost::asio::error::eof || ec == http::error::end_of_stream) {
                // клиент закрылся — выходим
                return;
            }
            if (ec) {
                std::cerr << "read_some(header): " << ec.message() << "\n";
                return;
            }
            header_raw.append(chunk, chunk + n);

            // лимит на размер заголовков
            if (header_raw.size() > header_limit) {
                beast::error_code ig;
                http::write(stream, make_error(http::status::request_header_fields_too_large, "header too large"), ig);
                return;
            }

            // нашли конец заголовков?
            if (header_raw.find("\r\n\r\n") != std::string::npos) {
                header_ok = true;
                break;
            }
        }
        if (!header_ok) return; // страховка

        // переливаем считанный заголовок в Beast-буфер и даём Beast допарсить
        {
            auto mb = buffer.prepare(header_raw.size());
            std::memcpy(mb.data(), header_raw.data(), header_raw.size());
            buffer.commit(header_raw.size());
        }

        http::request_parser<http::string_body> parser;
        if (MAX_BODY > 0) parser.body_limit((std::uint64_t)MAX_BODY);
        if (MAX_HDR  > 0) parser.header_limit((std::uint64_t)MAX_HDR);
        parser.eager(false); // тело не тянем заранее

        // парсим заголовок из буфера (без чтения из сокета)
        http::read_header(stream, buffer, parser, ec);
        if (ec == http::error::partial_message) {
            // теоретически не должны сюда попасть (мы уже нашли \r\n\r\n), но на всякий случай
            beast::error_code ig;
            http::write(stream, make_error(http::status::request_timeout, "read timeout (header-parse)"), ig);
            return;
        }
        if (ec == http::error::header_limit) {
            beast::error_code ig; http::write(stream, make_error(http::status::request_header_fields_too_large, "header too large"), ig); 
            return;
        }
        if (ec) { std::cerr << "read_header(parse): " << ec.message() << "\n"; return; }

        // --- 2) если требуется тело — дочитываем его С ДОП. ДЕДЛАЙНОМ ---
        {
            const auto& req_hdrs = parser.get();
            const bool might_have_body =
                (req_hdrs.method() != http::verb::get && req_hdrs.method() != http::verb::head) &&
                (parser.content_length().has_value() ? parser.content_length().value() > 0 : true);

            if (might_have_body) {
                auto t1 = std::chrono::steady_clock::now();
                while (!parser.is_done()) {
                    int remain = READ_TIMEOUT_MS - (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t1).count();
                    if (READ_TIMEOUT_MS > 0 && remain <= 0) {
                        beast::error_code ig;
                        http::write(stream, make_error(http::status::request_timeout, "read timeout (body)"), ig);
                        return;
                    }
                    if (!wait_readable(stream.socket(), READ_TIMEOUT_MS > 0 ? remain : 0)) {
                        beast::error_code ig;
                        http::write(stream, make_error(http::status::request_timeout, "read timeout (body)"), ig);
                        return;
                    }
                    std::size_t n = stream.socket().read_some(buffer.prepare(1024), ec);
                    if (ec == boost::asio::error::eof || ec == http::error::end_of_stream) return;
                    if (ec) { std::cerr << "read_some(body): " << ec.message() << "\n"; return; }
                    buffer.commit(n);
                    http::read(stream, buffer, parser, ec); // доедаем из buffer (read может не читать сокет, если буфер полон)
                    if (ec == http::error::body_limit) {
                        beast::error_code ig; http::write(stream, make_error(http::status::payload_too_large, "body too large"), ig); 
                        return;
                    }
                    if (ec && ec != http::error::need_buffer) { std::cerr << "read(body): " << ec.message() << "\n"; return; }
                }
            }
        }

        auto req = parser.get();

        // --- 3) формируем ответ и шлём с коротким дедлайном ---
        stream.expires_after(std::chrono::seconds(2));
        auto res = dispatch(req);
        res.version(req.version());
        res.keep_alive(req.keep_alive());

        ++handled;
        if (MAX_REQS > 0 && handled >= MAX_REQS) res.keep_alive(false);

        beast::error_code ecw;
        http::write(stream, res, ecw);
        stream.expires_never();

        if (ecw) { std::cerr << "write: " << ecw.message() << "\n"; break; }
        if (!res.keep_alive() || res.need_eof()) break;
    }

    beast::error_code ec;
    stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
}




int main(){
    try{
        auto addr = net::ip::make_address("0.0.0.0");
        unsigned short port = 8080;
        if (const char* p = std::getenv("PORT")) port = (unsigned short)std::stoi(p);

        std::cout << "Mega Auth Service on " << port
                  << " | build=" << BUILD_SIG
                  << " | READ=" << READ_TIMEOUT_MS << "ms"
                  << " IDLE=" << IDLE_TIMEOUT_S << "s"
                  << " MAX_REQS=" << MAX_REQS
                  << std::endl;

        net::io_context ioc{1};
        tcp::acceptor acc{ioc, {addr, port}};

        for(;;){
            tcp::socket s{ioc};
            acc.accept(s);
            std::thread{std::move(session), std::move(s)}.detach();
        }
    } catch (const std::exception& e){
        std::cerr << "Fatal: " << e.what() << std::endl; return 1;
    }
    return 0;
}
