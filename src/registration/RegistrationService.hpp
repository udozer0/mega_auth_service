#pragma once
#include <pqxx/pqxx>
#include "RegistrationMethod.hpp"
#include "RegistrationFactory.hpp"
#include "../infra/db.hpp"

class RegistrationService {
public:
    RegistrationService(DbPool& pool, FactoryDeps deps);
    RegistrationResult Handle(const RegistrationRequest& req);
private:
    DbPool& _pool;
    FactoryDeps _deps;
};
