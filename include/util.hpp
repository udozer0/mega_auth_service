
#pragma once
#include <boost/beast/http.hpp>
#include <string>
namespace http = boost::beast::http;
namespace util
{
inline void set_json_response( http::response<http::string_body>& r, std::string b, unsigned s = 200 )
{
     r.version( 11 );
     r.result( ( http::status )s );
     r.set( http::field::content_type, "application/json; charset=utf-8" );
     r.body() = std::move( b );
     r.prepare_payload();
}
inline void set_yaml_response( http::response<http::string_body>& r, std::string b, unsigned s = 200 )
{
     r.version( 11 );
     r.result( ( http::status )s );
     r.set( http::field::content_type, "application/yaml; charset=utf-8" );
     r.body() = std::move( b );
     r.prepare_payload();
}
inline void set_html_response( http::response<http::string_body>& r, std::string b, unsigned s = 200 )
{
     r.version( 11 );
     r.result( ( http::status )s );
     r.set( http::field::content_type, "text/html; charset=utf-8" );
     r.body() = std::move( b );
     r.prepare_payload();
}
} // namespace util
