#include "tradeabletoken.hpp"

void tradeableToken::init() {    
}

void tradeableToken::test() {    
}

void tradeableToken::buy(const account_name account, asset eos) {    
    auto market_itr = _market.begin();
    int64_t delta;
    eosio_assert(eos.amount > 0, "Must buy with positive Eos.");

    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(eos, HPY_SYMBOL).amount;
    });
    eosio_assert(delta > 0, "must reserve a positive amount");  
    asset hpy(delta, HPY_SYMBOL);
    issue(account, hpy, "issue some new hpy");
}

void tradeableToken::sell(const account_name account, asset hpy) {    
    require_auth(account);
    auto market_itr = _market.begin();
    int64_t delta;
    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(hpy, EOS_SYMBOL).amount;
    });
    eosio_assert(delta > 0, "Must burn a positive amount");
    // burn(account, hpy);
    asset eos(delta, EOS_SYMBOL);
    // transfer eos
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
                make_tuple(_self, account, eos, std::string("Sell happyeosslot.com share HPY.")))
        .send();
}

// @abi action
void tradeableToken::onTransfer(account_name from, account_name to, asset token, std::string memo) {        
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(token.is_valid(), "Invalid token transfer");
    eosio_assert(token.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(token.amount > 0, "must bet a positive amount");
    if (memo.find("buy") != string::npos) {
        buy(from, token);
    } else {
        sell(from, token);
    }
}


#define EOSIO_WAST(TYPE, MEMBERS)                                                                                  \
    extern "C"                                                                                                       \
    {                                                                                                                \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                \
        {                                                                                                            \
                                                                                                                     \
            auto self = receiver;                                                                                    \
            if (action == N(onerror))                                                                                \
            {                                                                                                        \
                eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
            }                                                                                                        \
            if (code == TOKEN_CONTRACT && action == N(transfer)) {                                                   \
                action = N(onTransfer);                                                                              \
            }                                                                                                        \
            if ((code == TOKEN_CONTRACT && action == N(onTransfer)) || code == self && action != N(onTransfer)) {                               \
                TYPE thiscontract(self);                                                                             \
                switch (action)                                                                                      \
                {                                                                                                    \
                    EOSIO_API(TYPE, MEMBERS)                                                                         \
                }                                                                                                     \
            }                                                                                                        \
        }                                                                                                            \
    }
// generate .wasm and .wast file
// EOSIO_WAST(tradeableToken, (onTransfer)(transfer)(init)(test))

// generate .abi file
EOSIO_ABI(tradeableToken, (transfer)(init)(test))