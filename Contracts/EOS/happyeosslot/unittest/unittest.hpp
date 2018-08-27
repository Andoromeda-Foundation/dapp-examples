/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include "../happyeosslot.hpp"

#define EOS_SYMBOL S(4, EOS)
#define HPY_SYMBOL S(4, HPY)
#define TOKEN_CONTRACT N(eosio.token)

typedef double real_type;

using std::string;
using eosio::symbol_name;
using eosio::asset;
using eosio::symbol_type;
using eosio::contract;
using eosio::permission_level;
using eosio::action;

class unittest : public contract {
    public:
        token( account_name self ):contract(self){}

        void test();

};