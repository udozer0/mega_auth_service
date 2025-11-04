#pragma once
#include <stdexcept>
struct DuplicateEmailError : std::runtime_error {
    using std::runtime_error::runtime_error;
};
