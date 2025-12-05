
#pragma once
#include <boost/beast/http.hpp>
#include <string_view>
#include <string>
#include "middleware/cors.hpp"
#include "handlers/health.hpp"
#include "infra/db.hpp"
#include "handlers/auth/auth_register.hpp"
#include "handlers/auth/auth_common.hpp"
#include "metrics.hpp"

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

inline http::response<http::string_body> route_request(
    const http::request<http::string_body>& req,
    DbPool& pool)
{
    auto target = path_only(to_std_sv(req.target()));

    // Preflight – можно не писать в метрики, пусть живёт отдельно
    if (req.method() == http::verb::options) {
        http::response<http::string_body> r;
        middleware::set_preflight(r);
        return r;
    }

    http::response<http::string_body> res;

    // ---- живые и заглушечные ручки ----

    if (target == "/health") {
        res = handlers::health::handle(req);
    }
    else if (req.method() == http::verb::post && target == "/auth/register") {
        res = auth_register_handle(req, pool);
    }
    else if (req.method() == http::verb::post && target == "/auth/login") {
        res = auth::make_bad(501, "not_implemented: /auth/login");
    }
    else if (req.method() == http::verb::post && target == "/auth/device/login") {
        res = auth::make_bad(501, "not_implemented: /auth/device/login");
    }
    else if (req.method() == http::verb::post && target == "/auth/forgot") {
        res = auth::make_bad(501, "not_implemented: /auth/forgot");
    }
    else if (req.method() == http::verb::post && target == "/auth/reset") {
        res = auth::make_bad(501, "not_implemented: /auth/reset");
    }
    else if (req.method() == http::verb::post && target == "/auth/change-password") {
        res = auth::make_bad(501, "not_implemented: /auth/change-password");
    }
    else if (req.method() == http::verb::post && target == "/auth/logout") {
        res = auth::make_bad(501, "not_implemented: /auth/logout");
    }
    else if (req.method() == http::verb::get && target == "/metrics") {
        // твой stub /metrics, который уже был
        mini_json::object o;
        o.set("status", "ok");
        o.set("message", "stub metrics");
        util::set_json_response(res, o.dump(), 200);
        middleware::apply_cors(res);
    }
    else {
        // неизвестный маршрут
        res = auth::make_bad(404, "not_found");
    }

    // ---- ОДНО место, где шлём метрики в StatsD ----
    try {
        metrics::track_request(std::string(target),
                               static_cast<int>(res.result_int()),
                               res.body().size());
    } catch (...) {
        // метрики никогда не должны ронять запрос, так что глушим
    }

    return res;
}
