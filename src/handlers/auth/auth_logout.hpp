
#pragma once
#include <boost/beast/http.hpp>
#include <string>
#include "auth_common.hpp"
namespace http=boost::beast::http; namespace handlers{ namespace auth{ http::response<http::string_body> auth_logout(const http::request<http::string_body>& req); } }
