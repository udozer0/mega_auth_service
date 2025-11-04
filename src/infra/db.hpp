#pragma once
#include <pqxx/pqxx>
#include <memory>
#include <string>

struct DbPool {
    std::string connstr;
};

std::unique_ptr<pqxx::connection> get_conn(const DbPool& pool);
