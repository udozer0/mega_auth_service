#pragma once
#include <memory>
#include "IRegistrationStrategy.hpp"
#include "PasswordRegistration.hpp"
#include "ClientCertRegistration.hpp"
#include "WebAuthnRegistration.hpp"

struct FactoryDeps {
    IUserRepo& userRepo;
    ICertRepo& certRepo;
    IEmailSender& email;
    IPkiService& pki;
    IWebAuthnService& webauthn;
    decltype(StrategyContext::opts) opts;
};

struct RegistrationFactory {
    static std::unique_ptr<IRegistrationStrategy>
    Create(RegistrationMethod method, const FactoryDeps&) {
        switch (method) {
            case RegistrationMethod::Password:   return std::make_unique<PasswordRegistration>();
            case RegistrationMethod::ClientCert: return std::make_unique<ClientCertRegistration>();
            case RegistrationMethod::WebAuthn:   return std::make_unique<WebAuthnRegistration>();
        }
        throw std::invalid_argument("unknown registration method");
    }
};
