#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>


using namespace eosio;

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

class underworld: public contract {

public:
        underworld(account_name self);

	void modifyprice(account_name from, uint64_t wei, int64_t amount);

	void transfer(account_name from, account_name to, asset quantity, std::string memo);  

	uint64_t balanceof(account_name who) const ;

	uint64_t getprice(uint64_t wei) const;

private:
        void _transfer(account_name account, asset eos);
        // @abi table player i64
        struct player {
         account_name account;
         uint64_t coin;
         uint64_t primary_key() const { return account; }
         EOSLIB_SERIALIZE(player, (account)(coin))
        };
        typedef eosio::multi_index<N(player), player> player_index;
        player_index players;
        struct price {
		
	   uint64_t wei;
	   uint64_t amount;
	   uint64_t primary_key() const { return wei; }
	   EOSLIB_SERIALIZE(price, (wei)(amount))		
       };
       typedef eosio::multi_index<N(price), price> price_index;
       price_index prices;
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
EOSIO_ABI_PRO(underworld, (modifyprice)(transfer))
//EOSIO_ABI(underworld, (modifyprice)(transfer))

underworld::underworld(account_name self): 
contract(self),
players(_self, _self),
prices(_self, _self){
}
void underworld::modifyprice(account_name from, uint64_t wei, int64_t amount) {

   require_auth(_self);
   auto p = prices.find(wei);
   if (p == prices.end()) { // Player already exist
      p = prices.emplace(_self, [&](auto& price){
         price.wei = wei;
	        price.amount = 0;
      });    
   }
   prices.modify(p, 0, [&](auto &price) {
      price.amount = amount;
   }); 
}

void underworld::transfer(account_name from, account_name to, asset quantity, std::string memo) { 
    if (from == _self || to != _self) {
      return;
    }
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "Quantity must be positive");
    if (quantity.symbol == EOS_SYMBOL) {
       _transfer(from, quantity);
     }
 }  

 void underworld::_transfer(account_name account, asset eos) {
   
   require_auth(account);
   eosio_assert(eos.amount > 0, "must purchase a positive amount");
   eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed" );    
   // 0.0001 EOS -> eos.amount =1
   auto pr = prices.find(eos.amount);
   if (pr == prices.end()) { //price not exist
	    return;
   }		
   auto p = players.find(account);
   if (p == players.end()) { 
      p = players.emplace(_self, [&](auto& player){
         player.account = account;
	       player.coin = 0;
      });    
   }
	
   players.modify(p, 0, [&](auto &player) {
      player.coin += pr->amount;
   });   
}


uint64_t underworld::balanceof(account_name who) const {
	
   auto p = players.find(who);
   if (p == players.end()) { // Player not exist
      return 0;   
   }
	
   return p->coin; 
}

uint64_t underworld::getprice(uint64_t wei) const {
	
   auto p = prices.find(wei);
   if (p == prices.end()) { 
      return 0;   
   }
   return p->amount;
}