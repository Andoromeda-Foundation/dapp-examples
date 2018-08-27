#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;;

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

class crptoherooo : public contract {
  	public:

  	crptoherooo(account_name self)
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

  // @abi table card i64
  struct card {
    uint64_t     id;
    account_name owner;
    uint64_t primary_key() const { return id; }
    EOSLIB_SERIALIZE(player, (id)(owner))
  };
  typedef eosio::multi_index<N(card), card> card_index;
  card_index cards;  

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
// EOSIO_ABI_PRO(crptoherooo, (transfer)(init)(sell)(bet)(reveal))

// generate .abi file
EOSIO_ABI(crptoherooo, (transfer)(init)(sell)(bet)(reveal))
