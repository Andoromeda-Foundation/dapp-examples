/**
 *  @dev minakokojima
 */

#include "eosotcbackup.hpp"

/// @abi action
void eosotcbackup::init() {
    require_auth(_self);    
}

/// @abi action
void eosotcbackup::test() {
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
void eosotcbackup::take(account_name owner, uint64_t order_id, extended_asset bid, extended_asset ask) {
    order_index orders(_self, bid.contract); 
    auto itr = orders.find(order_id);  
    
    eosio_assert(itr != orders.end(), "order is not exist.");
    //eosio_assert(itr->bid == ask, "ask is not equal to order bid.");
    //eosio_assert(itr->ask == bid, "bid is not equal to order ask.");

    // partial take
    eosio_assert( uint128_t(itr->bid.amount) * ask.amount == uint128_t(itr->ask.amount) * bid.amount, 
        "the price is not equal.");
        
    // extended_asset can only be used in eosio.token.

    if (itr->bid.amount <= ask.amount) {
        asset _bid = itr->bid;
        asset _ask = itr->ask;
        _bid.amount = 1;
        _ask.amount = 1;

        // 奖 bid 交给买家        
        action(
            permission_level{_self, N(active)},
            ask.contract, N(transfer),
            make_tuple(_self, owner, _bid,
                std::string("trade success"))
        ).send();

        // 将 ask 交给卖家
        action(
            permission_level{_self, N(active)},
            bid.contract, N(transfer),
            make_tuple(_self, itr->owner, _ask,
                std::string("trade success"))
        ).send();        

        insert_txlog(itr->owner, owner, itr->bid, itr->ask);
        orders.erase(itr);
    } else {
        asset _bid = ask;
        asset _ask = bid;

        // 奖 bid 交给买家           
        action(
            permission_level{_self, N(active)},
            ask.contract, N(transfer),
            make_tuple(_self, owner, _bid,
                std::string("trade success"))
        ).send();

        // 将 ask 交给卖家
        action(
            permission_level{_self, N(active)},
            bid.contract, N(transfer),
            make_tuple(_self, itr->owner, _ask,
                std::string("trade success"))
        ).send();

        orders.modify(itr, 0, [&](auto &o) {
            o.bid.amount -= _ask.amount;
            o.ask.amount -= _bid.amount;
        });
        insert_txlog(itr->owner, owner, bid, ask);
    }  
}

// @abi action
void eosotcbackup::retrieve(account_name owner, uint64_t order_id, extended_asset ask) {
    order_index orders(_self, ask.contract); 
    auto itr = orders.find(order_id);    
    eosio_assert(itr != orders.end(), "order is not exist.");
    eosio_assert(itr->owner == owner, "not the owner.");

    asset _bid = itr->bid;

    action(
        permission_level{_self, N(active)},
        itr->bid.contract, N(transfer),
        make_tuple(_self, owner, _bid,
            std::string("order retrieve"))
    ).send();
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
        _ask.symbol = string_to_symbol(s-f-1, memo.substr(s+1, s-p-1).c_str());

        eosio_assert(_ask.is_valid(), "invalid token transfer");
        eosio_assert(_ask.amount > 0, "must bet a positive amount");
        memo.erase(0, p+1);
        auto issuer = string_to_name(memo.c_str());
        _ask.contract = issuer;
        ask(from, bid, _ask);
    } else if (memo.substr(0, 4) == "take"){	
        memo.erase(0, 5);

    // memo
    // take,1.0000 HPY,happyeosslot,7

        std::size_t p = memo.find(',');
        std::size_t f = memo.find('.');  
        std::size_t s = memo.find(' ');   

        extended_asset _ask;
        _ask.amount = string_to_price(memo.substr(0, s));
        _ask.symbol = string_to_symbol(s-f-1, memo.substr(s+1, p-s-1).c_str());
    
        eosio_assert(_ask.is_valid(), "invalid token transfer");
        eosio_assert(_ask.amount > 0, "must bet a positive amount");
        memo.erase(0, p+1);

        p = memo.find(',');

        
        auto issuer = string_to_name(memo.substr(0, p).c_str());
        _ask.contract = issuer;
        memo.erase(0, p+1);
        auto id = string_to_price(memo);        
        take(from, id, bid, _ask);
    }
}

// @abi action
void eosotcbackup::cleantxlogs(uint64_t cnt) {
    auto timestamp_index = txlogs.get_index<N(bytimestamp)>();
    while (timestamp_index.begin() != timestamp_index.end()) {
        if (cnt-- == 0) return;
        timestamp_index.erase(timestamp_index.begin());
    }
}

void eosotcbackup::insert_txlog(account_name bidder, account_name asker, extended_asset bid, extended_asset ask) {
    txlogs.emplace(_self, [&](auto& t){
        t.id = txlogs.available_primary_key();
        t.bidder = bidder;
        t.asker = asker;    
        t.bid = bid;
        t.ask = ask;        
        t.timestamp = current_time();
    });
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

struct cleantxlogs_args
{
    uint64_t cnt;
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
        } else if (action == N(retrieve)){ 
            auto t = unpack_action_data<retrieve_args>();
            thiscontract.retrieve(t.owner, t.order_id, t.ask);
        } else if (action == N(cleantxlogs)){
            auto t = unpack_action_data<cleantxlogs_args>();
            thiscontract.cleantxlogs(t.cnt);
        } else {                                                                      
            /*switch (action)                                                                                      
            {                                                                                                    
                EOSIO_API(eosotcbackup, (retrieve)(init)(test)(cleantxlogs))                         
            } */             
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
/// EOSIO_WAST(eosotcbackup, (onTransfer)(retrieve)(init)(test)(cleantxlogs))

// generate .abi file
// EOSIO_ABI(eosotcbackup, (transfer)(retrieve)(init)(test)(cleantxlogs))