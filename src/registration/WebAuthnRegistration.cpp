#include "WebAuthnRegistration.hpp"
#include "../utils/validation.hpp"
#include "../infra/interfaces.hpp"
#include <stdexcept>

RegistrationResult WebAuthnRegistration::Register(const StrategyContext& ctx,
                                                  const RegistrationRequest& req) {
    if (!req.accept_terms) throw std::runtime_error("terms_not_accepted");
    if (!is_valid_email(req.email)) throw std::runtime_error("email_invalid");

    auto wa = ctx.services.webauthn.VerifyAttestation(
        req.webauthn_client_data_json, req.webauthn_attestation);

    auto [user_id, needs_verify] =
        ctx.services.userRepo.CreateUserWithoutPassword(ctx.tx, to_lower_trim(req.email));

    ctx.services.webauthn.BindCredential(ctx.tx, user_id, wa.credentialId, wa.publicKeyJwk);

    if (needs_verify && ctx.opts.require_email_verification) {
        ctx.services.email.SendVerifyEmail(req.email);
    }
    return { user_id, needs_verify, "" };
}
