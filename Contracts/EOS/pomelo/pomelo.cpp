/**
 *  @dev minakokojima@andoromeda
 */

#include "pomelo.hpp"


/// @abi action
void pomelo::init() {
    require_auth(_self);    

    while (buyrecords.begin() != buyrecords.end()) {
	    buyrecords.erase(buyrecords.begin());
    }
    while (sellrecords.begin() != sellrecords.end()) {
    	sellrecords.erase(sellrecords.begin());
    }        
    while (txlogs.begin() != txlogs.end()) {
	    txlogs.erase(txlogs.begin());
    }
}

/// @abi action
void pomelo::test() {

}

/// @abi action
void pomelo::cancelsell(account_name account, uint64_t id) {
    require_auth(account);
    auto itr = sellrecords.find(id);
    eosio_assert(itr->account == account, "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");
    // TODO: 返还
    sellrecords.erase(itr);
}

/// @abi action
void pomelo::cancelbuy(account_name account, uint64_t id) {
    require_auth(account);
    auto itr = buyrecords.find(id);
    eosio_assert(itr->account == account, "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");
    // TODO: 返还
    buyrecords.erase(itr);
}

/// @abi action
void pomelo::buy(account_name account, asset income, uint64_t target_symbol, uint64_t target_price) {
    // 生成购买订单
    buyrecord b;
    /*b.account = account;
    b.target = quant;
    b.per = target_price;
    b.total_eos = total_eos;*/
    do_buy_trade(b);
}

/// @abi action
void pomelo::sell(account_name account, asset quant, uint64_t total_eos)
{
    require_auth(account);
    eosio_assert(quant.symbol != EOS, "Must sale non-EOS currency");
    eosio_assert(total_eos > 0, "");

    action(
        permission_level{ account, N(active) },
        TOKEN_CONTRACT, N(transfer),
        make_tuple(account, _self, quant, string("transfer"))) // 由合约账号代为管理欲出售的代币
        .send();

    // 生成出售订单
    sellrecord s;
    s.account = account;
    s.asset = quant;
    s.per = (double)total_eos / (double)quant.amount;
    s.total_eos = total_eos;
    do_sell_trade(s);
}



uint64_t string_to_price(string s) {
    uint64_t z = 0;
    for (int i=0;i<s.size();++i) {
        if ('0' <= s[i] && s[i] <= '9') {
            z *= 10; 
            z += s[i];
        }
    }
    return z;
}

// memo [buy,HPY,1.23] EOS
// memo [sell,HPY,1.23] HPY
// @abi action
void pomelo::onTransfer(account_name from, account_name to, asset income, std::string memo) {        
    if (to != _self) return;    
    require_auth(from);
    eosio_assert(income.is_valid(), "invalid token transfer");
    eosio_assert(income.amount > 0, "must bet a positive amount");

    if (memo.substr(0, 3) == "buy") {
        eosio_assert(income.symbol == EOS, "only EOS allowed");
        memo.erase(0, 4);
        std::size_t p = memo.find(','); 
        auto target_symbol = string_to_symbol(4, memo.substr(0, p).c_str());        
        memo.erase(0, p+1);
        auto target_price = string_to_price(memo);
        buy(from, income, target_symbol, target_price);
    } else {	
        // sell 
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
            }                                            \
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
EOSIO_WAST(pomelo, (onTransfer)(cancelbuy)(cancelsell)(buy)(sell))

// generate .abi file
// EOSIO_ABI(pomelo, (cancelbuy)(cancelsell)(buy)(sell))