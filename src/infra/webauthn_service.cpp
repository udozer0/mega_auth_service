#include "webauthn_service.hpp"
#include <pqxx/pqxx>

IWebAuthnService::Attested StubWebAuthnService::VerifyAttestation(
    const std::string& client_data_json, const std::string& attestation) {
    return {"cred-id-demo", "{\"kty\":\"OKP\",\"crv\":\"Ed25519\",\"x\":\"...\"}"};
}

void StubWebAuthnService::BindCredential(pqxx::work& tx, const std::string& user_id,
                                         const std::string& credId, const std::string& pubJwk) {
    tx.exec_params(
        "INSERT INTO user_webauthn(user_id, credential_id, public_key_jwk) VALUES ($1,$2,$3)",
        user_id, credId, pubJwk);
}
