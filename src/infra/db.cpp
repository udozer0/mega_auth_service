#include "db.hpp"

std::unique_ptr<pqxx::connection> get_conn(const DbPool& pool) {
    return std::make_unique<pqxx::connection>(pool.connstr);
}
