
#pragma once
#include <boost/beast/http.hpp>
#include <string_view>
#include <string>
#include "middleware/cors.hpp"
#include "infra/db.hpp"
#include "handlers/auth/auth_register.hpp"
namespace http = boost::beast::http;

inline bool is_options( const http::request<http::string_body>& req )
{
     return req.method() == http::verb::options;
}
inline std::string_view to_std_sv(boost::beast::string_view v) {
    return std::string_view{ v.data(), v.size() };
}
inline std::string path_only(std::string_view t) {
    auto q = t.find('?');
    return q == std::string_view::npos ? std::string(t) : std::string(t.substr(0, q));
}

inline http::response<http::string_body> route_request(const http::request<http::string_body>& req, DbPool& pool)
{
     auto target = path_only(to_std_sv(req.target()));
    if (is_options(req)) {
        http::response<http::string_body> pre{http::status::no_content, req.version()};
        pre.set(http::field::access_control_allow_origin, "*");
        pre.set(http::field::access_control_allow_headers, "Content-Type, Authorization");
        pre.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
        return pre;
    }

     if (req.method() == http::verb::post && target == "/auth/register") {
        auto res = auth_register_handle(req, pool);  // ВЕСЬ разбор делает хендлер
        // CORS-хедеры добавим тут
        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_headers, "Content-Type, Authorization");
        res.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
        return res;
    }
     if (req.method() == http::verb::get && target == "/health") {
          http::response<http::string_body> ok{http::status::ok, req.version()};
          ok.set(http::field::content_type, "application/json; charset=utf-8");
          ok.body() = R"({"status":"ok"})";
          ok.prepare_payload();
          ok.set(http::field::access_control_allow_origin, "*");
          ok.set(http::field::access_control_allow_headers, "Content-Type, Authorization");
          ok.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
          return ok;
     }

    http::response<http::string_body> notf{http::status::not_found, req.version()};
    notf.set(http::field::content_type, "application/json; charset=utf-8");
    notf.body() = R"({"error":"not_found"})";
    notf.prepare_payload();
    return notf;
}
