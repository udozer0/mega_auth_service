#pragma once
#include <string>
#include <cstdint>

struct ArgonParams {
    uint32_t opslimit;
    size_t   memlimit;
    int      alg;
};

std::string hash_password(const std::string& password, const ArgonParams& p);
bool verify_password(const std::string& password, const std::string& stored);
