#include "cert_repo.hpp"
#include <pqxx/pqxx>

void PgCertRepo::AttachClientCert(pqxx::work& tx, const std::string& user_id,
                                  const std::string& serial, const std::string& fingerprint,
                                  const std::string& pem, const std::string& expires_at_iso) {
tx.exec_params(
    "INSERT INTO user_certs(user_id, serial, subject_dn, public_key, issued_at, expires_at, revoked, meta_json) "
    "VALUES ($1,$2,'',$3, now(), $4::timestamptz, false, jsonb_build_object('pem', $5::text))",
    user_id, serial, fingerprint, expires_at_iso, pem);
}
