/**
 *  @dev minakokojima
 */

#include "pomelo.hpp"


/// @abi action
void pomelo::init() {
    require_auth(_self);    

/*
    while (buyorders.begin() != buyorders.end()) {
	    buyorders.erase(buyorders.begin());
    }
    while (sellorders.begin() != sellorders.end()) {
    	sellorders.erase(sellorders.begin());
    }        
    while (txlogs.begin() != txlogs.end()) {
	    txlogs.erase(txlogs.begin());
    }*/
}

/// @abi action
void pomelo::test() {

}

/// @abi action
void pomelo::cancelsell(account_name issuer, account_name account, uint64_t id) {
    require_auth(account);
    auto itr = sellorders.find(id);
    eosio_assert(itr->account == account, "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");
    // TODO: 返还 DONE:@yukiexe

            action(
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, sell_itr->account, itr->bid,
                std::string("trade cancel successed"))
        ).send(); 
/*
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, itr->account, itr->bid,
            std::string("trade success"))
    ).send();*/

    
    sellorders.erase(itr);
}

/// @abi action
void pomelo::cancelbuy(account_name issuer, account_name account, uint64_t id) {
    require_auth(account);
    buyorder_index buyorders(_self, issuer);    
    auto itr = buyorders.find(id); 
    eosio_assert(itr != buyorders.end(), "Trade id is not found");
    eosio_assert(itr->account == account, "Account does not match");

    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, itr->account, itr->bid,
            std::string("trade cancel successed"))
    ).send();    

    buyorders.erase(itr);
}

/// @abi action
void pomelo::buy(account_name account, asset bid, asset ask, account_name issuer) {
    // 生成购买订单

/*    //eosio_assert(false, "emmm");
    static char msg[30];
    sprintf(msg, "%d %d\n", bid.amount, ask.amount);
    eosio_assert(false, msg);
    return;*/

    buyorder o;
    o.account = account;
    o.bid = bid;
    o.ask = ask;
    // do_buy_trade(b);

    buyorder_index buyorders(_self, issuer);    
    buyorders.emplace(_self, [&](auto& t) {    
        t.id = buyorders.available_primary_key();
        t.account = account;
        t.bid = bid;
        t.ask = ask;      
        t.timestamp = now();
    });
}

/// @abi action
void pomelo::sell(account_name account, asset bid, asset ask, account_name issuer) {
    sellorder o;
    o.account = account;
    o.bid = bid;
    o.ask = ask;
}



uint64_t string_to_price(string s) {
    uint64_t z = 0;
    for (int i=0;i<s.size();++i) {
        if ('0' <= s[i] && s[i] <= '9') {
            z *= 10; 
            z += s[i] - '0';
        }
    }
    return z;
}

// @abi action
void pomelo::match(account_name issuer, uint64_t buy_id, uint64_t sell_id) {

    buyorder_index buyorders(_self, issuer);   
    sellorder_index sellorders(_self, issuer);   
    auto buy_itr = buyorders.find(buy_id);
    auto sell_itr = sellorders.find(sell_id);   
    
    if (uint128_t(buy_itr->ask.amount) * sell_itr->bid.amount == uint128_t(sell_itr->ask.amount) * buy_itr->bid.amount) { // to be refine, avoid use div

        auto price = buy_itr->get_price();
        uint64_t delta = std::min(uint64_t(buy_itr->bid.amount), uint64_t(sell_itr->bid.amount * price)); 

        action(
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, sell_itr->account, asset(delta, EOS),
                std::string("trade success"))
        ).send();

        action(
            permission_level{_self, N(active)},
            N(buy_itr->issuer), N(transfer),
            make_tuple(_self, buy_itr->account, asset(delta, sell_itr->bid.symbol),
                std::string("trade success"))
        ).send();        

        if (buy_itr->bid.amount - delta == 0) {
            buyorders.erase(buy_itr);
        } else {
            buyorders.modify(buy_itr, 0, [&](auto &o) {
                o.bid.amount -= delta;
                o.ask.amount -= delta / price;
            });
        }

        if (sell_itr->bid.amount - delta == 0) {
            sellorders.erase(sell_itr);
        } else {
            sellorders.modify(sell_itr, 0, [&](auto &o) {
                o.bid.amount -= delta / price;
                o.ask.amount -= delta;
            });
        }
    }    
}

void pomelo::transfer(account_name from, account_name to, asset bid, std::string memo) { 
    return;
}

// memo [buy,issuer,HPY,1.23] EOS
// memo [sell,issuer,HPY,1.23] HPY
// @abi action
void pomelo::onTransfer(account_name from, account_name to, asset bid, std::string memo) {        
    if (to != _self) return;    
    require_auth(from);
    eosio_assert(bid.is_valid(), "invalid token transfer");
    eosio_assert(bid.amount > 0, "must bet a positive amount");


    if (memo.substr(0, 3) == "buy") {

        eosio_assert(bid.symbol == EOS, "only EOS allowed");
        memo.erase(0, 4);
        std::size_t p = memo.find(','); 

/*    static char msg[30];
    sprintf(msg, "%s", memo.substr(0, p).c_str());
    eosio_assert(false, msg);
    return;        */

  
        auto issuer = string_to_name(memo.substr(0, p).c_str());
        memo.erase(0, p+1);

        p = memo.find(',');         
        auto ask_symbol = string_to_symbol(4, memo.substr(0, p).c_str());        
        memo.erase(0, p+1);
        auto ask_price = string_to_price(memo);
        buy(from, bid, asset(ask_price, EOS), issuer);
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
EOSIO_WAST(pomelo, (onTransfer)(cancelbuy)(cancelsell)(buy)(sell))

// generate .abi file
// EOSIO_ABI(pomelo, (transfer)(cancelbuy)(cancelsell)(buy)(sell)(match))