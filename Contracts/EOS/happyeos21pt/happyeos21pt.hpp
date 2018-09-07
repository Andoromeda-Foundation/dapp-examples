/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
//#include "../eosio.token/eosio.token.hpp"
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

class happyeos21pt : public contract {
    public:
        happyeos21pt(account_name self):
            contract(self),
            global(_self, _self),
            games(_self, _self) {}

        void init(const checksum256& hash);
        void test();
        
        void reveal(checksum256 &seed, const uint32_t dPoints, const uint32_t pPoints, const checksum256 &hash);
        void onTransfer(account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo);              
        
    private:
        void join(const account_name player, const account_name referal, asset eos, const checksum256& seed);
        void hit(const account_name player);
        void stand(const account_name player);
        // void surrender(const account_name player);
        void merge_seed(checksum256 &s1, checksum256 s2);
        checksum256 parse_memo(const std::string &memo) const;
        void add_bet(const account_name player, const checksum256& seed, const asset eos);
            
    public:  
        struct account {
            asset    balance;
            uint64_t primary_key() const { return balance.symbol.name(); }
        };
        typedef eosio::multi_index<N(accounts), account> accounts; 

    private:
        // @abi table global i64
        struct global {
            uint64_t id = 0;
            checksum256 hash; // hash of the game seed, 0 when idle.

            auto primary_key() const { return id; }
            EOSLIB_SERIALIZE(global, (id)(hash)) 
        };
        typedef eosio::multi_index<N(global), global> global_index;
        global_index global;          

        // @abi table game i64
        struct game {
            uint64_t id;
            account_name player;
            uint64_t bid;
            checksum256 seed;

            auto primary_key() const { return id; }
            EOSLIB_SERIALIZE(game, (id)(player)(bid)(seed))
        };
        typedef eosio::multi_index<N(game), game> game_index;
        game_index games;   

        
    private:
        void send_referal_bonus(const account_name referal, asset eos);    
        bool isplayer_win(const uint32_t dPoints, const uint32_t pPoints);
        void deal_with(game_index::const_iterator itr, const uint32_t dPoints, const uint32_t pPoints);

        bool verify(const checksum256 &seed, const uint32_t dPoints, const uint32_t pPoints);
 
     };
