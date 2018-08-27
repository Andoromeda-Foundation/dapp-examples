#include <eosiolib/crypto.h>
#include <cstdio>
#include "cryptoherooo.hpp"

// @abi action
void cryptoherooo::init(const checksum256& hash) {
    require_auth(_self);

	global.begin();

    auto g = global.find(0);

    if (g == global.end()) {
        global.emplace(_self, [&](auto &g) {
            g.id = 0;
			g.draw_price = 1;
            g.hash = hash;
        });
    } else {
        global.erase(global.begin());

        global.emplace(_self, [&](auto &g) {
            g.id = 0;
			g.draw_price = 1;
            g.hash = hash;
        });
    }
}

// @abi action
void cryptoherooo::test(const account_name account, asset eos) {

}

// @abi action
void cryptoherooo::draw(const account_name account, asset eos, const checksum256& seed) {
    offers.emplace(_self, [&](auto& offer) {
        offer.id = offers.available_primary_key();
        offer.owner = account;
        offer.count = 1;
        offer.seed = seed;
    });  
}

uint64_t merge_seed(const checksum256 &s1, const checksum256 &s2) {
    uint64_t hash = 0, x;
    for (int i = 0; i < 32; ++i) {
        hash ^= (s1.hash[i] ^ s2.hash[i]) << ((i & 7) << 3);
    }
    return hash;
}

uint64_t get_type(uint64_t seed) {
    /*seed %= 100000;
    int i = 0;
    while (seed >= p[i]) {
        seed -= p[i];
        ++i;
    }
    return b[i];*/
    return 0;
}

// @abi action
void cryptoherooo::issuecard(account_name to, uint64_t type_id, string memo) {
    require_auth(_self);
	
	uint64_t token_id = cards.available_primary_key();
	
    cards.emplace(_self, [&](auto& c) {
        c.id = token_id;
		c.type = type_id;
        c.owner = to;
    });
}

// @abi action
void cryptoherooo::transfercard(account_name from,
                                account_name to,
                                uint64_t     id,
                                string       memo) {
    require_auth(from);
    eosio_assert(from != to, "cannot transfer card to self" );     
    auto itr = cards.find(id);
    eosio_assert(itr->owner == from, "user don't have this card" );
    cards.modify(itr, 0, [&](auto &c) {
        c.owner = to;
    });      
    require_recipient(from);
    require_recipient(to);      
}

void cryptoherooo::_reveal(eosio::multi_index<N(offer), offer>::const_iterator itr, const checksum256 &seed) {
    uint64_t type_id = get_type(merge_seed(seed, itr->seed));
    
    /*static char msg[10];
    sprintf(msg, "card type: %d", type_id);*/
	
    action(
        permission_level{_self, N(active)},
        _self, N(issuecard),
        std::make_tuple(itr->owner, type_id, ""))
    .send();

    offers.erase(itr);  
}

// @abi action
void cryptoherooo::reveal(const checksum256 &seed, const checksum256 &hash) {
    require_auth(_self);
    assert_sha256((char *)&seed, sizeof(seed), (const checksum256 *)&global.begin()->hash);
    auto n = offers.available_primary_key();
    for (int i = 0; i < n; ++i) {
        auto itr = offers.find(i);
        _reveal(itr, seed);
    }
    auto itr = global.find(0);
    global.modify(itr, 0, [&](auto &g) {
        g.hash = hash;
    });  
}

checksum256 parse_memo(const std::string &memo) { // to bo refine.
    checksum256 checksum;
    memset(&checksum, 0, sizeof(checksum256));
    for (int i = 0; i < memo.length(); i++) {
        checksum.hash[i & 31] ^= memo[i];
    }
    return checksum;
}

// @abi action
void cryptoherooo::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");

    if (memo.find("draw") != string::npos) {
        const checksum256 seed = parse_memo(memo);
        draw(from, eos, seed);	
    } else {	
        //buy(from, eos);      
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
EOSIO_WAST(cryptoherooo, (onTransfer)(issuecard)(transfercard)(init)(test)(reveal))

// generate .abi file
// EOSIO_ABI(cryptoherooo, (issuecard)(transfercard)(init)(test)(reveal))
/* transfer() add by hand*/

