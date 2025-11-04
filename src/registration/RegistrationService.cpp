#include "RegistrationService.hpp"
#include "IRegistrationStrategy.hpp"

RegistrationService::RegistrationService(DbPool& pool, FactoryDeps deps)
: _pool(pool), _deps(deps) {}

RegistrationResult RegistrationService::Handle(const RegistrationRequest& req) {
    auto conn = get_conn(_pool);
    pqxx::work tx(*conn);

    StrategyContext ctx{
        tx,
        { _deps.userRepo, _deps.certRepo, _deps.email, _deps.pki, _deps.webauthn },
        _deps.opts
    };

    auto strat = RegistrationFactory::Create(req.method, _deps);
    auto result = strat->Register(ctx, req);

    // audit/metrics hooks could be here
    tx.commit();
    return result;
}
