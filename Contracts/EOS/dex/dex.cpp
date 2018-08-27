#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;;

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

class dex : public contract {
  	public:

  	dex(account_name self)
		: contract(self),
        offers(_self, _self),      
        players(_self, _self),
        global(_self, _self) {
  	}

    void init(account_name self, const checksum256& hash) {
    }

    void transfer(account_name from, account_name to, asset quantity, std::string memo) { // I cannot understand this...
      if (from == _self || to != _self) {
        return;
      }
      eosio_assert(quantity.is_valid(), "Invalid token transfer");
      eosio_assert(quantity.amount > 0, "Quantity must be positive");
      _transfer(from, quantity);
    }

    void _transfer(account_name account, asset eos) {
  
    }

    void take(const account_name host, const checksum256& seed, const checksum256& hash) {

    }
      
    uint64_t get_credits(account_name account) const {
      const auto& p = players.get(account);
      return p.credits;
    }

    private:
    // @abi table global i64
    struct global {
      uint64_t id = 0;
      checksum256 hash; // hash of the game seed, 0 when idle.

      uint64_t primary_key() const { return id; }
      EOSLIB_SERIALIZE(global, (id)(hash))
    };
    typedef eosio::multi_index<N(global), global> global_index;
    global_index global;  

    // @abi table player i64
    struct player {
      account_name account;
      uint64_t credits;

      uint64_t primary_key() const { return account; }
      EOSLIB_SERIALIZE(player, (account)(credits))
    };
    typedef eosio::multi_index<N(player), player> player_index;
    player_index players;

    // @abi table offer i64
    struct offer {
        uint64_t          id;
        account_name      owner;
        uint64_t          bet;
        checksum256       seed;

        uint64_t primary_key()const { return id; }
        EOSLIB_SERIALIZE( offer, (id)(owner)(bet)(seed) )
    };  
    typedef eosio::multi_index< N(offer), offer> offer_index;  
    offer_index offers;

    const int p[8] = {25,50,1200,1000,4000,20000,50000,23725};
    const float b[8] = {100, 50, 20, 10, 5, 2, 0.1, 0.01};

    float get_bonus(uint64_t seed) {
      seed %= 100000;
      int i = 0;
      while (seed > p[i]) {
        seed -= p[i];      
        ++i;
      }
      return b[i];
    }

    uint64_t merge_seed(const checksum256& s1, const checksum256& s2) {
      uint64_t hash = 0, x;
      for (int i = 0; i < 32; ++i) {
        hash ^= (s1.hash[i] ^ s2.hash[i]) << ((i & 7) << 3);
      }
      return hash;
    }

    void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr, const checksum256& seed) {
      auto p = players.find(itr->owner);
      eosio_assert(p != players.end(), "Player is not exist.");
      players.modify(p, 0, [&](auto &player) {
        player.credits += uint64_t(get_bonus(merge_seed(seed, itr->seed)) * itr->bet);
      });
      offers.erase(itr);
    }
};

#define EOSIO_ABI_PRO(TYPE, MEMBERS)                                                                                                              \
  extern "C" {                                                                                                                                    \
  void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                                                   \
  {                                                                                                                                               \
    auto self = receiver;                                                                                                                         \
    if (action == N(onerror))                                                                                                                     \
    {                                                                                                                                             \
      eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account");                                        \
    }                                                                                                                                             \
    if ((code == TOKEN_CONTRACT && action == N(transfer)) || (code == self && (action != N(transfer) ))) \
    {                                                                                                                                             \
      TYPE thiscontract(self);                                                                                                                    \
      switch (action)                                                                                                                             \
      {                                                                                                                                           \
        EOSIO_API(TYPE, MEMBERS)                                                                                                                  \
      }                                                                                                                                           \
    }                                                                                                                                             \
  }                                                                                                                                               \
  }

// generate .wasm and .wast file
EOSIO_ABI_PRO(slot_machine, (transfer)(init)(sell)(bet)(reveal))

// generate .abi file
// EOSIO_ABI(slot_machine, (transfer)(init)(sell)(bet)(reveal))