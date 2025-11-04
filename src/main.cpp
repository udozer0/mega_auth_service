#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>
#include <poll.h> // <= для wait_readable()

#include "router.hpp"
#include "infra/db.hpp"
#include "middleware/cors.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

static int env_int( const char* n, int def )
{
     if ( const char* v = std::getenv( n ) )
     {
          try
          {
               return std::stoi( v );
          }
          catch ( ... )
          {
          }
     }
     return def;
}

static int READ_TIMEOUT_MS = env_int( "READ_TIMEOUT_MS", 300 );
static int IDLE_TIMEOUT_S = env_int( "IDLE_TIMEOUT_S", 3 );
static int MAX_REQS = env_int( "MAX_REQS", 5 );
static int MAX_BODY = env_int( "MAX_BODY", 1024 );
static int MAX_HDR = env_int( "MAX_HDR", 8 * 1024 );

// сигнатура сборки — видно в /_meta и в баннере при старте
static const char* BUILD_SIG = __DATE__ " " __TIME__;

static http::response<http::string_body> make_error( http::status st, std::string msg )
{
     http::response<http::string_body> res;
     res.result( st );
     res.version( 11 );
     res.set( http::field::content_type, "application/json; charset=utf-8" );
     res.keep_alive( false );
     res.body() = std::string( "{\"status\":\"error\",\"message\":\"" ) + msg + "\"}";
     middleware::apply_cors( res );
     res.prepare_payload();
     return res;
}

http::response<http::string_body> dispatch(http::request<http::string_body>& req, DbPool& pool) {
    return route_request(req, pool);  // всё
}

// helper: ждём готовность к чтению через poll()
static bool wait_readable( boost::asio::ip::tcp::socket& s, int timeout_ms )
{
     if ( timeout_ms <= 0 )
          return true;
     struct pollfd pfd
     {
     };
     pfd.fd = s.native_handle();
     pfd.events = POLLIN;
     int r = ::poll( &pfd, 1, timeout_ms );
     if ( r <= 0 )
          return false; // ошибка или таймаут — считаем «нет данных»
     return ( pfd.revents & ( POLLIN | POLLERR | POLLHUP | POLLNVAL ) ) != 0;
}

void session(tcp::socket socket, DbPool& pool)
{
    beast::tcp_stream stream{std::move(socket)};
    beast::flat_buffer buffer;

    for (;;) {
        // 1) читаем весь запрос (заголовки + тело) с дедлайном
        stream.expires_after(std::chrono::seconds(5)); // можно вынести в env
        http::request<http::string_body> req;
        beast::error_code ec;
        http::read(stream, buffer, req, ec);
        stream.expires_never();

        if (ec == http::error::end_of_stream || ec == boost::asio::error::eof) {
            // клиент закрыл соединение — выходим без ошибок
            break;
        }
        if (ec) {
            // любые другие ошибки протокола
            beast::error_code ig;
            http::write(stream, make_error(http::status::bad_request, ec.message()), ig);
            break;
        }

        // 2) диспатч и ответ
        auto res = dispatch(req, pool);
        res.version(req.version());
        res.keep_alive(req.keep_alive());

        beast::error_code ecw;
        http::write(stream, res, ecw);
        if (ecw) {
            // не смогли отправить — закрываемся
            break;
        }

        if (!res.keep_alive() || res.need_eof()) {
            // по протоколу надо закрыть
            break;
        }
    }

    beast::error_code ig;
    stream.socket().shutdown(tcp::socket::shutdown_send, ig);
}


int main()
{
     try
     {
          const char* v = std::getenv("PM_CONNSTR");
          std::string connstr = v ? v : "host=localhost port=5432 dbname=pm user=pm password=pm";
          DbPool pool{ connstr };
          auto addr = net::ip::make_address( "0.0.0.0" );
          unsigned short port = 8080;
          if ( const char* p = std::getenv( "PORT" ) )
               port = ( unsigned short )std::stoi( p );

          std::cout << "Mega Auth Service on " << port << " | build=" << BUILD_SIG << " | READ=" << READ_TIMEOUT_MS
                    << "ms"
                    << " IDLE=" << IDLE_TIMEOUT_S << "s"
                    << " MAX_REQS=" << MAX_REQS << std::endl;

          net::io_context ioc { 1 };
          tcp::acceptor acc { ioc, { addr, port } };
          for (;;) {
               tcp::socket s{ ioc };
               acc.accept(s);
               std::thread([&pool](tcp::socket sock){
                    session(std::move(sock), pool);
               }, std::move(s)).detach();
          }
     }
     catch ( const std::exception& e )
     {
          std::cerr << "Fatal: " << e.what() << std::endl;
          return 1;
     }
     return 0;
}
