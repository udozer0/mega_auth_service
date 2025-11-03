
#pragma once
#include <boost/beast/http.hpp>
#include <string_view>
#include <string>
#include "middleware/cors.hpp"
namespace http=boost::beast::http;
inline bool is_options(const http::request<http::string_body>& req){ return req.method()==http::verb::options; }
inline std::string path_only(std::string_view t){ auto q=t.find('?'); if(q==std::string_view::npos) return std::string(t); return std::string(t.substr(0,q)); }
