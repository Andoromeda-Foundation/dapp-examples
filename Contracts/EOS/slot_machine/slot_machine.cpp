#include <eosiolib/currency.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;
using namespace std;
using eosio::key256;

typedef double real_type;

#define GAME_SYMBOL S(4, "CREDITS")

class elot : public contract {
  public:

  elot(account_name self)
      : contract(self),
        offers(_self, _self),      
        global(_self, _self) {
    // Create a new global if not exists
    auto gl_itr = global.begin();
    if (gl_itr == global.end()) {
    };
  }

  void buy(account_name account, asset eos) {
    require_auth(account);
    eosio_assert(eos.amount > 0, "must purchase a positive amount");
  }

  void sell(account_name account, int64_t credits) {
  }
  
  void bet(const account_name player, const asset& bet, const checksum256& seed) {
    eosio_assert( bet.symbol == GAME_SYMBOL, "only core token allowed" );
    eosio_assert( bet.is_valid(), "invalid bet" );
    eosio_assert( bet.amount > 0, "must bet positive quantity" );    
    auto itr = offers.emplace(_self, [&](auto& offer){
      offer.id         = offers.available_primary_key();
      offer.bet        = bet;
      offer.owner      = player;
      offer.seed       = seed;
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

  void withdraw(const account_name host, const asset& credits) {

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

  //@abi table offer i64
  struct offer {
      uint64_t          id;
      account_name      owner;
      asset             bet;
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
// EOSIO_ABI_PRO(elot, (sell)(destroy)(claim))

// generate .abi file
// EOSIO_ABI(elot, (buy)(sell)(bet)(reveal)(withdraw))