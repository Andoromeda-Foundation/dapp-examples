/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include "../tradeabletoken/tradeabletoken.hpp"

#define EOS_SYMBOL S(4, EOS)
#define HPY_SYMBOL S(4, HPY)
#define TOKEN_CONTRACT N(eosio.token)

typedef double real_type;

class cryptoherooo : public contract {
    public:
    cryptoherooo( account_name self ):contract(self),
        global(_self, _self),    
        cards(_self, _self),
        offers(_self, _self) {}
        
    void init(const checksum256& hash);
    void test(const account_name account, asset eos);    

    void draw(const account_name account, asset eos, const checksum256& seed);
    void reveal(const checksum256 &seed, const checksum256 &hash);

    void issuecard(account_name to, uint64_t type_id, string memo);

    void transfercard(account_name from,
                      account_name to,
                      uint64_t     id,
                      string       memo);

    void onTransfer(account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo);    
    
    // @abi table global i64
    struct global {
        uint64_t id = 0;
        uint64_t draw_price = 0;
        checksum256 hash;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(global, (id)(draw_price)(hash))
    };
    typedef eosio::multi_index<N(global), global> global_index;
    global_index global;  
    // @abi table card i64
    struct card {
        uint64_t     id;
        uint64_t     type;
        account_name owner;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(card, (id)(type)(owner))
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
        EOSLIB_SERIALIZE(offer, (id)(owner)(count)(seed))
    };  
    typedef eosio::multi_index< N(offer), offer> offer_index;  
    offer_index offers;

    private:
    void _reveal(eosio::multi_index<N(offer), offer>::const_iterator itr, const checksum256 &seed);
};
