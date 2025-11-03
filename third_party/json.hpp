
#pragma once
#include <string>
#include <map>
#include <sstream>
namespace mini_json
{
struct object
{
     std::map<std::string, std::string> kv;
     void set( const std::string& k, const std::string& v )
     {
          kv[ k ] = v;
     }
     std::string dump() const
     {
          std::ostringstream os;
          os << "{";
          bool f = true;
          for ( auto& p : kv )
          {
               if ( !f )
                    os << ",";
               f = false;
               os << "\"" << p.first << "\":\"" << p.second << "\"";
          }
          os << "}";
          return os.str();
     }
};
} // namespace mini_json
