#include <eosiolib/crypto.h>

#include "happyeosslot.hpp"

void happyeosslot::init(account_name self, const checksum256 &hash) {
    eosio_assert(self == _self, "only contract itself.");
    auto g = global.find(0);
    if (g == global.end()) {
        global.emplace(_self, [&](auto &g) {
            g.id = 0;
            g.hash = hash;
        });
    } else {
        global.modify(g, 0, [&](auto &g) {
            g.hash = hash;
        });
    }
}

void happyeosslot::transfer(account_name from, account_name to, asset eos, std::string memo) {
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");

    const checksum256 seed = parse_memo(memo);

    offers.emplace(_self, [&](auto &offer) {
        offer.id = offers.available_primary_key();
        offer.owner = from;
        offer.bet = eos.amount;
        offer.seed = seed;
    });
    /*
    auto p = results.find(itr->owner);
    if (p == results.end()) {
        p = results.emplace(_self, [&](auto& result) {
            result.account = itr->owner;
            result.roll_result = 0;
        });
    } else {
        results.modify(p, 0, [&](auto& result) {
            results.roll_result = 0;
        });
    }*/
}

void happyeosslot::reveal(const account_name host, const checksum256 &seed, const checksum256 &hash) {
    require_auth(host);
    eosio_assert(host == _self, "Only happyeosslot can reveal the answer.");
    assert_sha256((char *)&seed, sizeof(seed), (const checksum256 *)&global.begin()->hash);
    auto n = offers.available_primary_key();
    for (int i = 0; i < n; ++i) {
        auto itr = offers.find(i);
        deal_with(itr, seed);
    }
    auto itr = global.find(0);
    global.modify(itr, 0, [&](auto &g) {
        g.hash = hash;
    });
}

#define EOSIO_ABI_PRO(TYPE, MEMBERS)                                                                                 \
    extern "C"                                                                                                       \
    {                                                                                                                \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                \
        {                                                                                                            \
            auto self = receiver;                                                                                    \
            if (action == N(onerror))                                                                                \
            {                                                                                                        \
                eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
            }                                                                                                        \
            if ((code == TOKEN_CONTRACT && action == N(transfer)) || (code == self && (action != N(transfer))))      \
            {                                                                                                        \
                TYPE thiscontract(self);                                                                             \
                switch (action)                                                                                      \
                {                                                                                                    \
                    EOSIO_API(TYPE, MEMBERS)                                                                         \
                }                                                                                                    \
            }                                                                                                        \
        }                                                                                                            \
    }

// generate .wasm and .wast file
EOSIO_ABI_PRO(happyeosslot, (transfer)(init)(reveal))

// generate .abi file
// EOSIO_ABI(slot_machine, (transfer)(init)(reveal))
