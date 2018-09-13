/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include "kyubey.hpp"
#include "utils.hpp"

#include <cmath>
#include <string>

typedef double real_type;

class happyeosdice : public kyubey {
    public:
        happyeosdice(account_name self):
            kyubey(self),
            global(_self, _self),
            offers(_self, _self) {}

        // @abi action
        void init(const checksum256& hash);
        // @abi action
        void test(const account_name account, asset eos);
        // @abi action
        void buy(const account_name account, asset eos);
        // @abi action        
        void sell(const account_name account, asset hpy);        
        
        // EOS transfer event.
        void onTransfer(account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo);                  

        void reveal( const checksum256 &seed, const checksum256 &hash);


        struct account {
            asset    balance;
            uint64_t primary_key() const { return balance.symbol.name(); }
        };
        typedef eosio::multi_index<N(accounts), account> accounts; 

        // @abi table global i64
        struct global {
            uint64_t id = 0;
            checksum256 hash; // hash of the game seed, 0 when idle.
            uint64_t offerBalance; // All balance in offer list.

            uint64_t primary_key() const { return id; }
            EOSLIB_SERIALIZE(global, (id)(hash)(offerBalance)) 
        };
        typedef eosio::multi_index<N(global), global> global_index;
        global_index global;          

        // @abi table offer i64
        struct offer {
            uint64_t id;
            account_name owner;
            uint64_t bet;
            uint64_t under;
            checksum256 seed;

            uint64_t primary_key() const { return id; }
            EOSLIB_SERIALIZE(offer, (id)(owner)(bet)(under)(seed))
        };
        typedef eosio::multi_index<N(offer), offer> offer_index;
        offer_index offers;

        // @abi table result i64
        struct result {
            uint64_t id;
            uint64_t roll_number;
            uint64_t primary_key() const { return id; }
            EOSLIB_SERIALIZE(result, (id)(roll_number))
        };
        typedef eosio::multi_index<N(result), result> results;

        
        void send_referal_bonus(const account_name referal, asset eos);
        void bet(const account_name account, const account_name referal, asset eos, const checksum256& seed, const uint64_t bet_number);
        void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr, const checksum256& seed);
        void set_roll_result(const account_name account, uint64_t roll_number);

        uint64_t get_bonus(uint64_t seed) const;
        uint64_t merge_seed(const checksum256& s1, const checksum256& s2) const;
        checksum256 parse_memo(const std::string &memo) const;
};
