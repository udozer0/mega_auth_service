#pragma once
#include <string>
#include <stdexcept>

enum class RegistrationMethod { Password, ClientCert, WebAuthn };

inline RegistrationMethod parse_method(const std::string& s) {
    if (s == "password") return RegistrationMethod::Password;
    if (s == "client_cert") return RegistrationMethod::ClientCert;
    if (s == "webauthn") return RegistrationMethod::WebAuthn;
    throw std::invalid_argument("unknown method");
}

struct RegistrationRequest {
    RegistrationMethod method;
    std::string email;
    std::string password;
    std::string csr_pem;
    std::string webauthn_client_data_json;
    std::string webauthn_attestation;
    bool accept_terms{false};
};

struct RegistrationResult {
    std::string user_id;
    bool email_verification_required{true};
    std::string issued_client_cert_pem;
};
