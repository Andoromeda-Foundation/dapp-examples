#include <utility>
#include <vector>
#include <string>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>

using eosio::key256;
using eosio::indexed_by;
using eosio::const_mem_fun;
using eosio::asset;
using eosio::permission_level;
using eosio::action;
using eosio::print;
using eosio::name;

typedef double real_type;

using namespace eosio;
using namespace std;

#define CORE_SYMBOL S(4, SYS)
#define TOKEN_CONTRACT N(eosio.token)

class slot_machine : public contract {
  	public:

  	slot_machine(account_name self)
		: contract(self),
        offers(_self, _self),      
        players(_self, _self),
        global(_self, _self) {
  	}

  void init(account_name self, const checksum256& hash) {
    eosio_assert(self == _self, "only contract itself."); 	  

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

  void transfer(account_name from, account_name to, asset quantity, string memo) { // I cannot understand this...
    eosio::print("Now counter is ", quantity.amount);        
    if (from == _self || to != _self) {
      return;
    }
    eosio::print("Now counter is ", quantity.amount);      
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "Quantity must be positive");
    eosio::print("Now counter is ", quantity.amount);          // only accepts CORE_SYMBOL for buy
    if (quantity.symbol == CORE_SYMBOL) {
      _transfer(from, quantity);
    }
  }  

  void _transfer(account_name account, asset eos) {
    eosio::print("Now counter is ", eos.amount);    
    require_auth(account);
    eosio::print("Now counter is ", eos.amount);        
    eosio_assert(eos.amount > 0, "must purchase a positive amount");
    eosio_assert(eos.symbol == CORE_SYMBOL, "only core token allowed" );    
    eosio::print("Now counter is ", eos.amount);    

    auto p = players.find(account);
    if (p == players.end()) { // Player already exist
      p = players.emplace(_self, [&](auto& player){
        player.account = account;
      });    
    }
    players.modify(p, 0, [&](auto &player) {
      player.credits += eos.amount * 1000;
    });
    eosio::print("Now counter is: ", p->credits);        
  }

  void sell(account_name account, int64_t credits) {
    require_auth(account);
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
        make_tuple(_self, account, asset(credits / 1000, CORE_SYMBOL), string("I'll be back.")))
        .send();     
  }

  void bet(const account_name account, const uint64_t bet, const checksum256& seed) {
    eosio::print("????\n");     
    require_auth(account);    
    auto p = players.find(account);
    eosio::print("Now credits is: ", p->credits); 

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
    eosio_assert(host == _self, "...");     
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
  }

  // In ponzi we trust.
  void withdraw(const account_name host, uint64_t value) {
    require_auth(host);
    eosio_assert(host == _self, "..."); 
    action(
      permission_level{_self, N(active)},
      N(eosio.token), N(transfer),
      make_tuple(_self, host, asset(value, CORE_SYMBOL), string("I'll be back."))
    ).send();      
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

  void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr, const checksum256& seed) {
    auto p = players.find(itr->owner);
    eosio_assert(p != players.end(), "player is not exist.");
    players.modify(p, 0, [&](auto &player) {
      player.credits += (uint64_t(get_bonus(seed.hash[0] ^ itr->seed.hash[0])) * itr->bet);
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
EOSIO_ABI_PRO(slot_machine, (transfer)(init)(sell)(bet)(reveal)(withdraw))

// generate .abi file
// EOSIO_ABI(slot_machine, (transfer)(init)(sell)(bet)(reveal)(withdraw))