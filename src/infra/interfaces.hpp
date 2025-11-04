#pragma once
#include <string>
#include <utility>
#include <pqxx/pqxx>

struct IUserRepo {
    virtual ~IUserRepo() = default;
    virtual std::pair<std::string,bool> CreateUserWithEmailVerification(
        pqxx::work&, const std::string& email,
        const std::string& password_hash, const std::string& algo) = 0;

    virtual std::pair<std::string,bool> CreateUserWithoutPassword(
        pqxx::work&, const std::string& email) = 0;
};

struct ICertRepo {
    virtual ~ICertRepo() = default;
    virtual void AttachClientCert(pqxx::work&, const std::string& user_id,
                                  const std::string& serial, const std::string& fingerprint,
                                  const std::string& pem, const std::string& expires_at_iso) = 0;
};

struct IEmailSender {
    virtual ~IEmailSender() = default;
    virtual void SendVerifyEmail(const std::string& email) = 0;
};

struct IPkiService {
    struct SignedCert { std::string serial, fingerprint, pem, expires_at; };
    virtual ~IPkiService() = default;
    virtual SignedCert SignClientCSR(const std::string& csr_pem) = 0;
};

struct IWebAuthnService {
    struct Attested { std::string credentialId, publicKeyJwk; };
    virtual ~IWebAuthnService() = default;
    virtual Attested VerifyAttestation(const std::string& client_data_json,
                                       const std::string& attestation) = 0;
    virtual void BindCredential(pqxx::work&, const std::string& user_id,
                                const std::string& credId, const std::string& pubJwk) = 0;
};
