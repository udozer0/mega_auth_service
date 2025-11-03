
#include "auth_register.hpp"
namespace handlers{ namespace auth{ http::response<http::string_body> auth_register(const http::request<http::string_body>& req){ if(req.method()!=http::verb::post) return ::auth::make_bad(405,"Register user: method must be POST"); return ::auth::make_ok("Register user accepted"); } } }
