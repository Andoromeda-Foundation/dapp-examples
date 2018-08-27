#include <eosiolib/crypto.h>
#include "happyeosslot.hpp"

void cryptohero::init() {
}

void cryptohero::test() {
}

// @abi action
void cryptohero::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");
     string operation = memo.substr(0, 3);
    if (operation == "buy") {
        buy(from, eos);      
    } else {
        //const checksum256 seed = parse_memo(memo);
        //draw(from, eos, seed);
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
EOSIO_WAST(happyeosslot, (onTransfer)(transfer)(init)(test))

// generate .abi file
// EOSIO_ABI(happyeosslot, (transfer)(init)(sell)(reveal)(test))