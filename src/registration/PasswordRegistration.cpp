#include "PasswordRegistration.hpp"
#include "../utils/validation.hpp"
#include "../domain/password_hash.hpp"
#include "../infra/interfaces.hpp"
#include <sodium.h>
#include <stdexcept>

RegistrationResult PasswordRegistration::Register(const StrategyContext& ctx,
                                                  const RegistrationRequest& req) {
    if (!req.accept_terms) throw std::runtime_error("terms_not_accepted");
    if (!is_valid_email(req.email)) throw std::runtime_error("email_invalid");
    if (!is_strong_password(req.password)) throw std::runtime_error("password_weak");

    ArgonParams params{3, 64*1024*1024, crypto_pwhash_ALG_ARGON2ID13};
    auto ph = hash_password(req.password, params);
    std::string algo = "argon2id:libsodium:v=19,m=65536,t=3,p=1";

    auto [user_id, needs_verify] =
        ctx.services.userRepo.CreateUserWithEmailVerification(
            ctx.tx, to_lower_trim(req.email), ph, algo);

    if (needs_verify && ctx.opts.require_email_verification) {
        ctx.services.email.SendVerifyEmail(req.email);
    }

    return { user_id, needs_verify, "" };
}
