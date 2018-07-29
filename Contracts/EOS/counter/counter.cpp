#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
using namespace eosio;

class counter_contract : public eosio::contract {
  public:
    counter_contract(account_name self) :eosio::contract(self){
        amount = 0;
    }
    using eosio::contract::contract;
    void add(account_name receiver) {
        amount += 1;
        eosio::print("Now amount is %d", amount);
    }
    void sub(account_name receiver){
        amount -= 1;
        eosio::print("Now amount is %d", amount);
    }
  private:
    uint64_t amount;
};

EOSIO_ABI( counter_contract, (add)(sub) )