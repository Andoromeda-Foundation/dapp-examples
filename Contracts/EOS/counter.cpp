// #include "counter.hpp"

using namespace eosio;

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

struct counter:public contract {

  using contract::contract;

  uint256 counter;

  /// @abi action
  void inc( account_name user ) {
    counter ++;
    print( "", name{user} );
  }

  /// @abi action
  uint256 get() { return counter; }
};

EOSIO_ABI(counter, (inc)(get))
