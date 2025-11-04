#pragma once
#include "IRegistrationStrategy.hpp"

class WebAuthnRegistration final : public IRegistrationStrategy {
public:
    RegistrationResult Register(const StrategyContext& ctx,
                                const RegistrationRequest& req) override;
};
