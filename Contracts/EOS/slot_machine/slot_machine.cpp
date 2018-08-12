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

#define GAME_SYMBOL S(4, CREDITS)
#define CORE_SYMBOL S(4, SYS)
#define TOKEN_CONTRACT N(eosio.token)

class elot : public contract {
  public:

  elot(account_name self)
      : contract(self),
        offers(_self, _self),      
        players(_self, _self),
        global(_self, _self) {
    // Create a new global if not exists
    auto gl_itr = global.begin();
    if (gl_itr == global.end()) {
    };
  }

  void transfer(account_name from, account_name to, asset quantity, string memo) { // I cannot understand this...
    if (from == _self || to != _self) {
      return;
    }
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "Quantity must be positive");
    // only accepts GAME_SYMBOL for buy
    if (quantity.symbol == GAME_SYMBOL) {
      buy(from, quantity);
    }
  }  

  void buy(account_name account, asset eos) {
    require_auth(account);
    eosio_assert(eos.amount > 0, "must purchase a positive amount");
    eosio_assert(eos.symbol == CORE_SYMBOL, "only core token allowed" );    

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
    require_auth(account);    
    auto p = players.find(account);
    eosio_assert(p->credits >= bet, "must have enouth credits");    
    players.modify(p, 0, [&](auto &player) {
      player.credits -= bet;
    });
    auto o = offers.emplace(_self, [&](auto& offer) {
      offer.id = offers.available_primary_key();
      offer.account = account;
      offer.bet = bet;
      offer.seed = seed;
    });     
  }

  void reveal(const account_name host, const checksum256& seed) {
    assert_sha256( (char *)&seed, sizeof(seed), (const checksum256 *)& global.begin()->hash );
    auto n = offers.available_primary_key();
    for (int i = 0; i < n; ++i) {
      auto itr = offers.find(i);
      deal_with(*itr);
    }
  }

  // In ponzi we trust.
  void withdraw(const account_name host) {
/*    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, account, asset(credits / 1000, CORE_SYMBOL), string("I'll be back.")))
        .send();       */
  }

  uint64_t get_credits(account_name acount) const {
    const auto& p = players.get(account);
    return p.credits;
  }

  private:
  // @abi table global i64
  struct global {
    uint64_t id = 0;
    uint64_t status; // 0: idle, 1: active
    checksum256 hash; // hash of the game seed

    uint64_t primary_key() const { return id; }
    EOSLIB_SERIALIZE(global, (id)(status)(hash))
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

  //@abi table offer i64
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

  void deal_with(const offer& itr) {
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
    if ((code == TOKEN_CONTRACT && action == N(transfer)) || (code == self && (action == N(sell) || action == N(destroy) || action == N(claim)))) \
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
// EOSIO_ABI_PRO(elot, (buy)(sell)(bet)(reveal)(withdraw))

// generate .abi file
EOSIO_ABI(elot, (buy)(sell)(bet)(reveal)(withdraw))