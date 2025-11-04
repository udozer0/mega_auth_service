#include "password_hash.hpp"
#include <sodium.h>
#include <stdexcept>

std::string hash_password(const std::string& password, const ArgonParams& p) {
    if (sodium_init() < 0) throw std::runtime_error("sodium_init failed");
    char out[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str_alg(out, password.c_str(), password.size(),
                              p.opslimit, p.memlimit, p.alg) != 0) {
        throw std::runtime_error("crypto_pwhash_str_alg failed");
    }
    return std::string(out);
}

bool verify_password(const std::string& password, const std::string& stored) {
    if (sodium_init() < 0) return false;
    return crypto_pwhash_str_verify(stored.c_str(), password.c_str(), password.size()) == 0;
}
