
#include "auth_login.hpp"
namespace handlers{ namespace auth{ http::response<http::string_body> auth_login(const http::request<http::string_body>& req){ if(req.method()!=http::verb::post) return ::auth::make_bad(405,"Login user: method must be POST"); return ::auth::make_ok("Login user accepted"); } } }
