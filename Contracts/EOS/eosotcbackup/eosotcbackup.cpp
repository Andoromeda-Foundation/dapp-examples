/**
 *  @dev minakokojima
 */

#include "eosotcbackup.hpp"


/// @abi action
void eosotcbackup::init() {
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
void eosotcbackup::test() {
    // eosio_assert(1==2,"go");
    // auto itr = buyorders.find(1);
    // buyorders.erase(itr);

/*    static char msg[30];
    sprintf(msg, "%s", memo.substr(0, p).c_str());
    eosio_assert(false, msg);
    return;        */
    /*
    buyorder_index buyorders(_self, N(happyeosslot));
    while (buyorders.begin() != buyorders.end()) {
        buyorders.erase(buyorders.begin());
        // eosio_assert(buyorders.begin() == buyorders.end(), "non empty.");
    }*/
    /*
    auto itr = buyorders.find(id);     
    if (global.begin() != global.end()) {
	    global.erase(global.begin());
    }
    if ( .begin() != _market.end()) {
	    _market.erase(_market.begin());
    }
    if (offers.begin() != offers.end()) {
	    offers.erase(offers.begin());
    }       
    */
}

/// @abi action
/*
void eosotcbackup::ask(account_name owner, asset bid, asset ask, account_name bid_issuer, account_name ask_issuer) {

}
*/

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

void eosotcbackup::ask(account_name owner, extended_asset bid, extended_asset ask) {
    order_index orders(_self, ask.contract);    
    orders.emplace(_self, [&](auto& o) {    
        o.id = orders.available_primary_key();
        o.owner = owner;
        o.bid = bid;
        o.ask = ask;
        o.timestamp = now();
    });
}
void eosotcbackup::take(account_name owner, uint64_t order_id, extended_asset ask) {
    order_index orders(_self, ask.contract); 
    auto itr = orders.find(order_id);    
    eosio_assert(itr != orders.end(), "order is not exist.");
    eosio_assert(itr->ask == ask, "ask is not equal.");
    // todo(minakokojima): 打钱
    orders.erase(itr);
}

void eosotcbackup::retrieve(account_name owner, uint64_t order_id, extended_asset ask) {
    order_index orders(_self, ask.contract); 
    auto itr = orders.find(order_id);    
    eosio_assert(itr != orders.end(), "order is not exist.");
    eosio_assert(itr->owner == owner, "not the owner.");
    eosio_assert(itr->ask == ask, "ask is not equal.");
    // todo(minakokojima): 打钱
    orders.erase(itr);    
}

// memo [ask,0.5000 HPY,happyeosslot]
// memo [take,0.5000 HPY,happyeosslot,id]
// @abi action
void eosotcbackup::onTransfer(account_name from, account_name to, extended_asset bid, std::string memo) {        
    if (to != _self) return;
    require_auth(from);
    eosio_assert(bid.is_valid(), "invalid token transfer");
    eosio_assert(bid.amount > 0, "must bet a positive amount");
    if (memo.substr(0, 3) == "ask") {
        memo.erase(0, 4);

        std::size_t p = memo.find(',');
        std::size_t f = memo.find('.');  
        std::size_t s = memo.find(' ');   

        extended_asset _ask;
        _ask.amount = string_to_price(memo.substr(0, s));
        _ask.symbol = string_to_symbol(s-f-1, memo.substr(s+1, memo.size()).c_str());

        eosio_assert(_ask.is_valid(), "invalid token transfer");
        eosio_assert(_ask.amount > 0, "must bet a positive amount");
        memo.erase(0, p+1);
        auto issuer = string_to_name(memo.c_str());
        _ask.contract = issuer;
        ask(from, bid, _ask);
    } else if (memo.substr(0, 4) == "take"){	
        
    }      
}


struct transfer_args
{
    account_name from;
    account_name to;
    asset quantity;
    string memo;
};

struct retrieve_args
{
    account_name owner;
    uint64_t order_id;
    extended_asset ask;
};

extern "C"
{
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {
        auto self = receiver;
        eosotcbackup thiscontract(self);
        if (action == N(transfer)) {
            auto transfer_data = unpack_action_data<transfer_args>();
            thiscontract.onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, code), transfer_data.memo);
        } else if (action == N(retrieve)) {
            auto retrieve_data = unpack_action_data<retrieve_args>();
            thiscontract.retrieve(retrieve_data.owner, retrieve_data.order_id, retrieve_data.ask);
        }
    }
} 
// #define EOSIO_WAST(TYPE, MEMBERS) apply(uint64_t receiver, uint64_t code, uint64_t action)
/*
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
            if (action == N(transfer)) {                                                   \
                 auto transfer_data = unpack_action_data<transfer_args>(); \
                 eosotcbbackup this_contract(self); \
                 this_contract.ontransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, code), transfer_data.memo);                                                                              \
            }                                                                                                        \
            else if ((code == TOKEN_CONTRACT && action == N(onTransfer)) || code == self && action != N(onTransfer)) {                               \
                TYPE thiscontract(self);                                                                             \
                switch (action)                                                                                      \
                {                                                                                                    \
                    EOSIO_API(TYPE, MEMBERS)                                                                         \
                }                                                                                                     \
            }                                                                                                        \
        }                                                                                                            \
    }
    */

// generate .wasm and .wast file
/// EOSIO_WAST(eosotcbackup, (onTransfer)(retrieve)(init)(test))

// generate .abi file
// EOSIO_ABI(eosotcbackup, (transfer)(init)(test))