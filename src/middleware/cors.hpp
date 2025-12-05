/// file cors.hpp
#pragma once
#include <boost/beast/http.hpp>
namespace http = boost::beast::http;
namespace middleware
{
inline void apply_cors( http::response<http::string_body>& r, const std::string& o = "*" )
{
     r.set( http::field::access_control_allow_origin, o );
     r.set( http::field::access_control_allow_headers, "Content-Type, Authorization" );
     r.set( http::field::access_control_allow_methods, "GET, POST, PUT, PATCH, DELETE, OPTIONS" );
     r.set( http::field::access_control_allow_credentials, "true" );
}
inline void set_preflight( http::response<http::string_body>& r, const std::string& o = "*" )
{
     r.version( 11 );
     r.result( http::status::no_content );
     apply_cors( r, o );
}
} // namespace middleware
