
#include "auth_change_password.hpp"
namespace handlers{ namespace auth{ http::response<http::string_body> auth_change_password(const http::request<http::string_body>& req){ if(req.method()!=http::verb::post) return ::auth::make_bad(405,"Change password: method must be POST"); return ::auth::make_ok("Change password accepted"); } } }
