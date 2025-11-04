#include "pki_service.hpp"
#include <string>

IPkiService::SignedCert StubPkiService::SignClientCSR(const std::string& csr_pem) {
    // stub: return fake certificate bundle
    return {
        "SERIAL123",
        "fp:sha256:DEADBEEF",
        "-----BEGIN CERTIFICATE-----\\n...stub...\\n-----END CERTIFICATE-----\\n",
        "2099-12-31T23:59:59Z"
    };
}
