
#pragma once
#include <boost/beast/http.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include "../middleware/cors.hpp"
#include "../../include/util.hpp"
namespace http = boost::beast::http;
namespace handlers
{
namespace docs
{
inline bool slurp( const std::string& p, std::string& out )
{
     std::ifstream f( p, std::ios::binary );
     if ( !f )
          return false;
     std::ostringstream s;
     s << f.rdbuf();
     out = s.str();
     return true;
}
inline std::string find_yaml()
{
     std::string c;
     if ( slurp( "openapi/openapi.yaml", c ) )
          return c;
     if ( slurp( "../openapi/openapi.yaml", c ) )
          return c;
     if ( slurp( "./openapi.yaml", c ) )
          return c;
     return {};
}
inline http::response<http::string_body> openapi_yaml()
{
     std::string body = find_yaml();
     http::response<http::string_body> r;
     if ( body.empty() )
     {
          r.result( http::status::not_found );
          util::set_yaml_response( r, "# not found\n" );
     }
     else
     {
          util::set_yaml_response( r, std::move( body ) );
     }
     middleware::apply_cors( r );
     return r;
}
inline http::response<http::string_body> swagger_ui()
{
     static const char* html = R"HTML(
<!doctype html><html><head><meta charset="utf-8"/><title>Mega Auth Service Docs</title>
<link rel="stylesheet" href="https://unpkg.com/swagger-ui-dist/swagger-ui.css">
<style>body{margin:0;background:#fafafa}</style></head><body><div id="swagger"></div>
<script src="https://unpkg.com/swagger-ui-dist/swagger-ui-bundle.js"></script>
<script>window.ui=SwaggerUIBundle({ url:'/openapi.yaml', dom_id:'#swagger' });</script></body></html>
)HTML";
     http::response<http::string_body> r;
     util::set_html_response( r, html );
     middleware::apply_cors( r );
     return r;
}
} // namespace docs
} // namespace handlers
