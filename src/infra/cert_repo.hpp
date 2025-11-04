#pragma once
#include "interfaces.hpp"

struct PgCertRepo : public ICertRepo {
    void AttachClientCert(pqxx::work& tx, const std::string& user_id,
                          const std::string& serial, const std::string& fingerprint,
                          const std::string& pem, const std::string& expires_at_iso) override;
};
