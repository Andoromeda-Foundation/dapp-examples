#include <algorithm>
#include <eosiolib/transaction.hpp>
#include "eosio.token.hpp"
#include "types.hpp"

class fairdicegame : public contract {
   public:
    fairdicegame(account_name self)
        : contract(self),
          /*_bets(_self, _self),
          _fund_pool(_self, _self),
          _hash(_self, _self),
          _global(_self, _self)*/{};

    // void transfer(const account_name& from, const account_name& to, const asset& quantity, const string& memo);
};

/*extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
    fairdicegame thiscontract(receiver);

    if ((code == N(eosio.token)) && (action == N(transfer))) {
        execute_action(&thiscontract, &fairdicegame::transfer);
        return;
    }

    if (code != receiver) return;

    switch (action) { EOSIO_API(fairdicegame, (receipt)(reveal)) };
    eosio_exit(0);
}
}*/
