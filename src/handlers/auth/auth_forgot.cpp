
#include "auth_forgot.hpp"
namespace handlers{ namespace auth{ http::response<http::string_body> auth_forgot(const http::request<http::string_body>& req){ if(req.method()!=http::verb::post) return ::auth::make_bad(405,"Forgot password request: method must be POST"); return ::auth::make_ok("Forgot password request accepted"); } } }
