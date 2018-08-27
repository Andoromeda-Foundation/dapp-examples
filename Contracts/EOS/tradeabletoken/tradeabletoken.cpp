#include "tradeabletoken.hpp"

void tradeableToken::buy(const account_name account, asset eos) {
    /*
   auto market_itr = _market.begin();
    int64_t delta;
    eos.amount /= eop(eos);
    eosio_assert(eos.amount > 0, "Must buy with positive Eos.");

    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(eos, HPY_SYMBOL).amount;
    });
    eosio_assert(delta > 0, "must reserve a positive amount");  
    asset hpy(delta, HPY_SYMBOL);
    issue(account, hpy, "issue some new hpy");*/
}

// @abi action
void tradeableToken::sell(const account_name account, asset hpy) {
    /*
    require_auth(account);
    auto market_itr = _market.begin();
    int64_t delta;
    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(hpy, EOS_SYMBOL).amount;
    });
    delta *= eop(asset(0, EOS_SYMBOL));
    eosio_assert(delta > 0, "Must burn a positive amount");
    burn(account, hpy);
    asset eos(delta, EOS_SYMBOL);
    // transfer eos
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
                make_tuple(_self, account, eos, std::string("Sell happyeosslot.com share HPY.")))
        .send();*/
}


// generate .wasm and .wast file
// MY_EOSIO_ABI(happyeosslot, (onTransfer)(transfer)(init)(sell)(reveal))

// generate .abi file
EOSIO_ABI(tradeableToken, (buy))