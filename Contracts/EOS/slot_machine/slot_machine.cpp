#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

class slot_machine : public contract {
  	public:

  	slot_machine(account_name self)
		: contract(self),
        offers(_self, _self),      
        players(_self, _self),
        global(_self, _self) {
  	}

  void init(const checksum256& hash) {
    require_auth(_self);
    auto g = global.find(0);
    if (g == global.end()) {
      global.emplace(_self, [&](auto& g){
        g.id = 0;
        g.hash = hash;
      });        
    } else {
      global.modify(g, 0, [&](auto& g) {      
        g.hash = hash;
      });   
    }
  }

  void transfer(account_name from, account_name to, asset quantity, std::string memo) {
    if (from == _self || to != _self) {
      return;
    }
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "Quantity must be positive");
    if (quantity.symbol == EOS_SYMBOL) {
      _transfer(from, quantity);
    }
  }  

  void _transfer(account_name account, asset eos) {
    require_auth(account);
    eosio_assert(eos.amount > 0, "must purchase a positive amount");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed" );

    auto p = players.find(account);
    if (p == players.end()) { // Player already exist
      p = players.emplace(_self, [&](auto& player){
        player.account = account;
      });
    }
    players.modify(p, 0, [&](auto &player) {
      player.credits += eos.amount * 1000;
    });    
  }

  void sell(account_name account, int64_t credits) {
    eosio_assert(credits > 0, "must sell a positive amount");
    require_auth(account);
    auto p = players.find(account);
    eosio_assert(p->credits >= credits, "must have enouth credits");
    players.modify(p, 0, [&](auto &player) {
      player.credits -= credits;
    });
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, account, asset(credits / 1000, EOS_SYMBOL), std::string("I'll be back.")))
        .send();     
  }

  void bet(const account_name account, const uint64_t bet, const checksum256& seed) {
    require_auth(account);
    auto p = players.find(account);
    eosio_assert(p->credits >= bet, "must have enouth credits");    
    players.modify(p, 0, [&](auto &player) {
      player.credits -= bet;
    });
    auto o = offers.emplace(_self, [&](auto& offer) {
      offer.id = offers.available_primary_key();
      offer.owner = account;
      offer.bet = bet;
      offer.seed = seed;
    });     
  }

  void reveal(const account_name host, const checksum256& seed, const checksum256& hash) {
    require_auth(host);
    eosio_assert(host == _self, "Only happyeosslot can reveal the answer.");
    assert_sha256( (char *)&seed, sizeof(seed), (const checksum256 *)& global.begin()->hash );
    auto n = offers.available_primary_key();
    for (int i = 0; i < n; ++i) {
      auto itr = offers.find(i);
      deal_with(itr, seed);
    }
    auto itr = global.find(0);
    global.modify(itr, 0, [&](auto &g) {
      g.hash = hash;
    });
    /* refund */
    uint64_t t = 12;
    for (; players.begin() != players.end() ;) {
      t--;
      if (t == 0) break;
      auto itr = players.begin();
      int64_t credits = itr->credits;
      auto account = itr->account;
      if (credits >= 1000) {
	      action(
			      permission_level{_self, N(active)},
			      N(eosio.token), N(transfer),
			      make_tuple(_self, account, asset(credits / 1000, EOS_SYMBOL), std::string("Updating contract, credits refund.")))
		      .send();     
      }
      players.erase(itr);
    }
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

  const int p[8] = {   10,   20,  120, 1000, 4000, 20000, 60000, 53835};
  const int b[8] = {10000, 5000, 2000, 1000,  500,   200,    10,     1};

  uint64_t get_bonus(uint64_t seed, uint64_t amount) {
      seed %= 100000;
      int i = 0;
      while (seed >= p[i]) {
          seed -= p[i];
          ++i;
      }
      return b[i] * amount / 100;
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
      eosio_assert(p != players.end(), "player is not exist.");
      players.modify(p, 0, [&](auto &player) {
        player.credits += get_bonus(merge_seed(seed, itr->seed), itr->bet);
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
// EOSIO_ABI_PRO(slot_machine, (transfer)(init)(sell)(bet)(reveal))

// generate .abi file
EOSIO_ABI(slot_machine, (transfer)(init)(sell)
