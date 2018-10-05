#include <eosiolib/crypto.h>
#include <cstdio>
#include "eosslgbackup.hpp"

// @abi action
void eosslgbackup::test(const account_name account, asset eos){
    require_auth(_self);
}

// @abi action
void eosslgbackup::buy(const account_name account, asset eos){
    require_auth(account);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");
    _add_price(account, eos);
}

// @abi action
void eosslgbackup::sell(const account_name account, asset eos){
    require_auth(_self);
    _sub_price(account, eos);
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, account, eos, std::string("Sold bonus. Have Fun!")))
    .send();
}

// @abi action
void eosslgbackup::check(const account_name account, asset eos, string memo){
    require_auth(account);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, account, eos, std::string("Check return.")))
    .send();
}

// @abi action
void eosslgbackup::take(const account_name from, const account_name to, asset eos){
    require_auth(_self);
    _sub_price(from, eos);
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, to, eos, std::string("Winner bunus. Have fun!")))
    .send();
}

// @abi action
void eosslgbackup::bonus(const account_name account, asset eos){
    require_auth(_self);
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, account, eos, std::string("Bouns. Have fun!")))
    .send();
}

// @abi action
void eosslgbackup::burn(const account_name account, asset eos){
    require_auth(_self);
    _sub_price(account, eos);
}

void eosslgbackup::_add_price(const account_name account, asset eos){
    // while(players.begin() != players.end()){
    //     players.erase(players.begin());
    // }
    auto p = players.find(account);
    if (p == players.end()) { 
        players.emplace(_self, [&](auto& player){
            player.account = account;
            player.balance = eos;
        });    
    }else{ // Player already exist
        players.modify(p, 0, [&](auto &player) {
            player.balance.amount += eos.amount;
        }); 
    }
}

void eosslgbackup::_sub_price(const account_name account, asset eos){
    auto p = players.find(account);

    eosio_assert(p != players.end(), "Invalid Player.");
    eosio_assert(p->balance.amount >= eos.amount, "Not Enough eos");

    players.modify(p, 0, [&](auto &player) {
        player.balance.amount -= eos.amount;
    }); 
}

// @abi action
void eosslgbackup::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "Only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");

    buy(from, eos);	
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
// EOSIO_WAST(eosslgbackup, (onTransfer)(buy)(sell)(take)(bonus)(burn)(test)(check))

// generate .abi file
EOSIO_ABI(eosslgbackup, (buy)(sell)(take)(bonus)(burn)(test)(check))
/* onTransfer() add by hand*/

