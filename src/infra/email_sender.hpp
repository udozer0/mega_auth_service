#pragma once
#include "interfaces.hpp"
#include <iostream>

struct StdoutEmailSender : public IEmailSender {
    void SendVerifyEmail(const std::string& email) override {
        std::cerr << "[email] send verification to " << email << std::endl;
    }
};
