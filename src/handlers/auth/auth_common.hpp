
#pragma once
#include <boost/beast/http.hpp>
#include <string>
#include "../../middleware/cors.hpp"
#include "../../../include/util.hpp"
#include "../../../third_party/json.hpp"
namespace http = boost::beast::http;
namespace auth
{
inline http::response<http::string_body> make_ok( const std::string& m )
{
     mini_json::object o;
     o.set( "status", "ok" );
     o.set( "message", m );
     http::response<http::string_body> r;
     util::set_json_response( r, o.dump(), 200 );
     middleware::apply_cors( r );
     return r;
}
inline http::response<http::string_body> make_bad( unsigned s, const std::string& m )
{
     mini_json::object o;
     o.set( "status", "error" );
     o.set( "message", m );
     http::response<http::string_body> r;
     util::set_json_response( r, o.dump(), s );
     middleware::apply_cors( r );
     return r;
}
} // namespace auth
