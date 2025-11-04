#pragma once
#include <pqxx/pqxx>
#include "RegistrationMethod.hpp"

struct IUserRepo;
struct ICertRepo;
struct IEmailSender;
struct IPkiService;
struct IWebAuthnService;

struct StrategyContext {
    pqxx::work& tx;
    struct {
        IUserRepo& userRepo;
        ICertRepo& certRepo;
        IEmailSender& email;
        IPkiService& pki;
        IWebAuthnService& webauthn;
    } services;
    struct {
        bool require_email_verification{true};
    } opts;
};

struct IRegistrationStrategy {
    virtual ~IRegistrationStrategy() = default;
    virtual RegistrationResult Register(const StrategyContext& ctx,
                                        const RegistrationRequest& req) = 0;
};
