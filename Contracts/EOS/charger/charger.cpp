#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;;

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

class charger : public contract {
    public:

    charger(account_name self)
    : contract(self),
        offers(_self, _self) {
    }

    void transfer(account_name from, account_name to, asset quantity, std::string memo) { // I cannot understand this...
      if (from == _self || to != _self) {
        return;
      }
      eosio_assert(quantity.is_valid(), "Invalid token transfer");
      eosio_assert(quantity.amount > 0, "Quantity must be positive");
      if (quantity.symbol == EOS_SYMBOL) {
        _transfer(from, quantity, memo);
      }
    }

    void _transfer(account_name account, asset eos, std::string memo) {
      eosio_assert(eos.amount > 0, "must purchase a positive amount");
      eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed" );    

      auto o = offers.emplace(_self, [&](auto& offer) {
        offer.id = offers.available_primary_key();
        offer.timestamp = current_time();
        offer.memo = memo;
      }); 
    }

    void clear(account_name self, uint64_t time) {
      require_auth(self);
      eosio_assert(self == _self, "Only happyeosslot can reveal the answer.");

      auto n = offers.available_primary_key();
      while (true) {
        auto itr = offers.begin();
        if (itr == offers.end()) break;
        if (itr->timestamp > time) break;
        deal_with(itr);
      }
    }
  
    private:

    // @abi table offer i64
    struct offer {
        uint64_t id;
        uint64_t timestamp;
        std::string memo;

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE( offer, (id)(memo) )
    };  
    typedef eosio::multi_index<N(offer), offer> offer_index;  
    offer_index offers;

    void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr) {
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
EOSIO_ABI_PRO(charger, (transfer)(clear))

// generate .abi file
// EOSIO_ABI(charger, (transfer)(clear))