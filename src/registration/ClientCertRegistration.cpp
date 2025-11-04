#include "ClientCertRegistration.hpp"
#include "../utils/validation.hpp"
#include "../infra/interfaces.hpp"
#include <stdexcept>

RegistrationResult ClientCertRegistration::Register(const StrategyContext& ctx,
                                                    const RegistrationRequest& req) {
    if (!req.accept_terms) throw std::runtime_error("terms_not_accepted");
    if (!is_valid_email(req.email)) throw std::runtime_error("email_invalid");
    if (req.csr_pem.empty()) throw std::runtime_error("csr_missing");

    auto [user_id, needs_verify] =
        ctx.services.userRepo.CreateUserWithoutPassword(ctx.tx, to_lower_trim(req.email));

    auto cert = ctx.services.pki.SignClientCSR(req.csr_pem);
    ctx.services.certRepo.AttachClientCert(ctx.tx, user_id, cert.serial, cert.fingerprint, cert.pem, cert.expires_at);

    if (needs_verify && ctx.opts.require_email_verification) {
        ctx.services.email.SendVerifyEmail(req.email);
    }

    RegistrationResult out{ user_id, needs_verify, "" };
    // If you generate keypair server-side and want to return PEM, put it here.
    return out;
}
