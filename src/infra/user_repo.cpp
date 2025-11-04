#include "user_repo.hpp"
#include "errors.hpp"
#include <pqxx/pqxx>
std::pair<std::string,bool> PgUserRepo::CreateUserWithEmailVerification(
    pqxx::work& tx, const std::string& email,
    const std::string& password_hash, const std::string& algo)
{
    try {
        auto r = tx.exec_params1(
            "INSERT INTO pm_users(email, password_hash, password_algo) "
            "VALUES ($1,$2,$3) RETURNING id",
            email, password_hash, algo);
        std::string user_id = r[0].c_str();

        tx.exec_params(
            "INSERT INTO pm_email_verifications(user_id, token, expires_at) "
            "VALUES ($1, gen_random_uuid()::text, now() + interval '24 hours')",
            user_id);

        return {user_id, true};
    } catch (const pqxx::unique_violation&) {
        throw DuplicateEmailError("email_already_exists");
    }
}

std::pair<std::string,bool> PgUserRepo::CreateUserWithoutPassword(
    pqxx::work& tx, const std::string& email)
{
    try {
        auto r = tx.exec_params1(
            "INSERT INTO pm_users(email, password_hash, password_algo) "
            "VALUES ($1,'','') RETURNING id",
            email);
        std::string user_id = r[0].c_str();

        tx.exec_params(
            "INSERT INTO pm_email_verifications(user_id, token, expires_at) "
            "VALUES ($1, gen_random_uuid()::text, now() + interval '24 hours')",
            user_id);

        return {user_id, true};
    } catch (const pqxx::unique_violation&) {
        throw DuplicateEmailError("email_already_exists");
    }
}
