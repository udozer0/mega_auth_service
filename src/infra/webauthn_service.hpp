#pragma once
#include "interfaces.hpp"

struct StubWebAuthnService : public IWebAuthnService {
    Attested VerifyAttestation(const std::string& client_data_json,
                               const std::string& attestation) override;
    void BindCredential(pqxx::work& tx, const std::string& user_id,
                        const std::string& credId, const std::string& pubJwk) override;
};
