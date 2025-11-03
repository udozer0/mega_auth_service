
#include "auth_logout.hpp"
namespace handlers
{
namespace auth
{
http::response<http::string_body> auth_logout( const http::request<http::string_body>& req )
{
     if ( req.method() != http::verb::post )
          return ::auth::make_bad( 405, "Logout: method must be POST" );
     return ::auth::make_ok( "Logout accepted" );
}
} // namespace auth
} // namespace handlers
