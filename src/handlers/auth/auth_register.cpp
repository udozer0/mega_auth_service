#include "auth_register.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "../../registration/RegistrationService.hpp"
#include "../../registration/RegistrationMethod.hpp"
#include "../../infra/interfaces.hpp"
#include "../../infra/user_repo.hpp"
#include "../../infra/cert_repo.hpp"
#include "../../infra/email_sender.hpp"
#include "../../infra/pki_service.hpp"
#include "../../infra/webauthn_service.hpp"
#include "../../infra/errors.hpp"

namespace http = boost::beast::http;
using json = nlohmann::json;

static json jerr(const std::string& code) { return json{{"error", code}}; }

http::response<http::string_body>
auth_register_handle(const http::request<http::string_body>& req, DbPool& pool)
{
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::content_type, "application/json; charset=utf-8");

    try {
        auto ct = req[http::field::content_type];
        if (ct.find("application/json") == std::string::npos)
            throw std::invalid_argument("bad_content_type");

        json body = json::parse(req.body(), nullptr, true);

        RegistrationRequest r{};
        r.method = parse_method(body.value("method", "password"));
        r.email  = body.value("email", "");
        r.password = body.value("password", "");
        r.csr_pem = body.value("csr_pem", "");
        r.webauthn_client_data_json = body.value("webauthn_client_data_json", "");
        r.webauthn_attestation      = body.value("webauthn_attestation", "");
        r.accept_terms = body.value("accept_terms", false);

        static PgUserRepo userRepo;
        static PgCertRepo certRepo;
        static StdoutEmailSender emailSender;
        static StubPkiService pkiService;
        static StubWebAuthnService webAuthnService;

        FactoryDeps deps{ userRepo, certRepo, emailSender, pkiService, webAuthnService, {/*opts*/ true} };
        RegistrationService service(pool, deps);
        auto rr = service.Handle(r);

        json out = { {"user_id", rr.user_id}, {"email_verification_required", rr.email_verification_required} };
        if (!rr.issued_client_cert_pem.empty()) out["issued_client_cert_pem"] = rr.issued_client_cert_pem;

        res.result(http::status::created);
        res.body() = out.dump();
    } catch (const DuplicateEmailError&) {
    res.result(http::status::conflict);
    res.body() = json{{"error","email_already_exists"}}.dump();
        } catch (const std::invalid_argument& e) {
            res.result(http::status::bad_request);
            res.body() = json{{"error", e.what()}}.dump();
        } catch (const std::exception& e) {
            res.result(http::status::bad_request);
            res.body() = json{{"error", e.what()}}.dump();
        }
    res.prepare_payload();
    return res;
}
