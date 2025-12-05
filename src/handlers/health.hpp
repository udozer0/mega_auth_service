/// file health.hpp
#pragma once
#include <boost/beast/http.hpp>
#include "../middleware/cors.hpp"
#include "../../include/util.hpp"
#include "../../third_party/json.hpp"
namespace http = boost::beast::http;
namespace handlers
{
namespace health
{
inline http::response<http::string_body> handle( const http::request<http::string_body>& req )
{
     if ( req.method() == http::verb::options )
     {
          http::response<http::string_body> r;
          middleware::set_preflight( r );
          return r;
     }
     mini_json::object o;
     o.set( "status", "UP" );
     http::response<http::string_body> r;
     util::set_json_response( r, o.dump(), 200 );
     middleware::apply_cors( r );
     return r;
}
} // namespace health
} // namespace handlers
