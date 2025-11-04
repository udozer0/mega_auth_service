#include <httplib.h>
#include "infra/db.hpp"
#include "handlers/auth_register.hpp"

void bind_routes(httplib::Server& app, const DbPool& pool) {
    bind_auth_register(app, pool);
}
