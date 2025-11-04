#pragma once
#include "interfaces.hpp"

struct StubPkiService : public IPkiService {
    SignedCert SignClientCSR(const std::string& csr_pem) override;
};
