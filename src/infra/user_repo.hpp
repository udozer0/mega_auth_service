#pragma once
#include "interfaces.hpp"

struct PgUserRepo : public IUserRepo {
    std::pair<std::string,bool> CreateUserWithEmailVerification(
        pqxx::work& tx, const std::string& email,
        const std::string& password_hash, const std::string& algo) override;

    std::pair<std::string,bool> CreateUserWithoutPassword(
        pqxx::work& tx, const std::string& email) override;
};
