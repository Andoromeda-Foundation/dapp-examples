/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include "../eosio.token/eosio.token.hpp"
#include <cmath>
#include <string>

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

class cryptoherooo : public tradeableToken {

    public:
    cryptoherooo( account_name self ):tradeableToken(self){}
    struct global {
        uint64_t id = 0;
        checksum256 hash;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(global, (id)(hash))
    };
    typedef eosio::multi_index<N(global), global> global_index;
    global_index global;  
    // @abi table global i64
    struct global {
        uint64_t id = 0;
        checksum256 hash;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(global, (id)(hash))
    };
    typedef eosio::multi_index<N(global), global> global_index;
    global_index global;  
    // @abi table card i64
    struct card {
        uint64_t     id;
        account_name owner;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(player, (id)(owner))
    };
    typedef eosio::multi_index<N(card), card> card_index;
    card_index cards;    
    // @abi table offer i64
    struct offer {
        uint64_t          id;
        account_name      owner;
        uint64_t          count;
        checksum256       seed;
        uint64_t primary_key()const { return id; }
        EOSLIB_SERIALIZE( offer, (id)(owner)(bet)(seed) )
    };  
    typedef eosio::multi_index< N(offer), offer> offer_index;  
    offer_index offers;
};
