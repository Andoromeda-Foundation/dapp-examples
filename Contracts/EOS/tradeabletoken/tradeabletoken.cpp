#include "tradeabletoken.hpp"


/*void tradeableToken::clear() {

}
*/

void tradeableToken::init(asset eos) {    

    require_auth(_self);
   
    if (_market.begin() != _market.end()) {
	    _market.erase(_market.begin());
    }
    
   /* stats statstable( _self, eos.symbol.name() );
    if (statstable.begin() != statstable.end()) {
	    statstable.erase(statstable.begin());
    }*/
    
    accounts minako(_self, N(minakokojima));
    while (minako.begin() != minako.end()) {
        minako.erase(minako.begin());
    }
    accounts necokeine(_self, N(iamnecokeine));
    while (necokeine.begin() != necokeine.end()) {
	    necokeine.erase(necokeine.begin());
    }

    require_auth( _self );
    if (_market.begin() == _market.end()) {
        _market.emplace(_self, [&](auto &m) {
            m.supply.amount = 2000000000000ll;
            m.supply.symbol = HPY_SYMBOL;
            m.deposit.balance.amount = init_quote_balance;
            m.deposit.balance.symbol = EOS_SYMBOL;
        });                
        create(_self, asset(210000000000ll, HPY_SYMBOL));
        issue(_self, asset(210000000000ll, HPY_SYMBOL), "");
    }
}

void tradeableToken::test(const account_name account, asset eos) {    

    // //eosio_assert(false, "emmm");
    // static char msg[100];
    // eosio_assert(false, msg);

    const auto& sym = eosio::symbol_type(HPY_SYMBOL).name();
    auto current_balance = asset(0, EOS_SYMBOL);
    buy(account, asset(10000, EOS_SYMBOL));
    
    eos.amount *=2;
    auto beforebuyamount1 = get_balance(account, sym).amount;
    buy(account, eos);
    auto delta = get_balance(account, sym).amount - beforebuyamount1;
    eosio_assert(delta > 0, "Delta should be positive.");

    sell(account, asset(delta, HPY_SYMBOL));
    auto afterbuysell1 = get_balance(account, sym).amount;
    eosio_assert(beforebuyamount1 == afterbuysell1, "not equal after sell1");

    // //auto beforebuyamount2 = get_balance(account, sym).amount;
    // eos.amount /= 2;
    // buy(account, eos);
    // //auto dd = get_balance(account, sym).amount;
    // //auto d3 = dd - beforebuyamount1;
    // buy(account, eos);
    // //auto delta2 = get_balance(account, sym).amount - dd;
    
    // //eosio_assert(delta >= delta2, "Buy one and Buy two");
    // //eosio_assert(delta - delta2 > 10, "not equal when buy 2 times.");
    // eosio_assert(false, "Test end");
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
    // issue(account, hpy, "issue some new hpy");
    action(
        permission_level{_self, N(active)},
        _self, N(transfer),
        make_tuple(_self, account, hpy, std::string("new token"))
    ).send(); 
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
EOSIO_WAST(tradeableToken, (onTransfer)(transfer)(init)(test))

// generate .abi file
// EOSIO_ABI(tradeableToken, (transfer)(issue)(init)(test))