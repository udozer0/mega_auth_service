
#include "auth_device_login.hpp"
namespace handlers{ namespace auth{ http::response<http::string_body> auth_device_login(const http::request<http::string_body>& req){ if(req.method()!=http::verb::post) return ::auth::make_bad(405,"Device login: method must be POST"); return ::auth::make_ok("Device login accepted"); } } }
