
#include "auth_reset.hpp"
namespace handlers{ namespace auth{ http::response<http::string_body> auth_reset(const http::request<http::string_body>& req){ if(req.method()!=http::verb::post) return ::auth::make_bad(405,"Reset password: method must be POST"); return ::auth::make_ok("Reset password accepted"); } } }
