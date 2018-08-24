/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>

using std::string;

class happyeosslot : public contract {
public:
    happyeosslot(account_name self):contract(self){}
    
    void create(account_name issuer, asset maximum_supply);
    
    void issue(account_name to, asset quantity, string memo);
    
    void transfer(account_name from,
                  account_name to,
                  asset        quantity,
                  string       memo);
    
    void sell(uint64_t amount);
    void reveal(const account_name host, const checksum256 &seed, const checksum256 &hash);
    
    inline asset get_supply(symbol_name sym)const;
    inline asset get_balance(account_name owner, symbol_name sym)const;
    
private:

    // @abi table global i64    
    struct account {
        asset    balance;
        uint64_t primary_key()const { return balance.symbol.name(); }
    };

    // @abi table global i64    
    struct currency_stats {
        asset          supply;
        asset          max_supply;
        account_name   issuer;
        
        uint64_t primary_key()const { return supply.symbol.name(); }
    };

    typedef eosio::multi_index<N(accounts), account> accounts;
    typedef eosio::multi_index<N(stat), currency_stats> stats;
        
    // @abi table global i64
    struct global {
        uint64_t id = 0;
        checksum256 hash; // hash of the game seed, 0 when idle.

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(global, (id)(hash))
    };
    typedef eosio::multi_index<N(global), global> global_index;
    global_index global;  

    // @abi table offer i64
    struct offer {
        uint64_t id;
        account_name owner;
        uint64_t bet;
        checksum256 seed;

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(offer, (id)(owner)(bet)(seed))
    };
    typedef eosio::multi_index<N(offer), offer> offer_index;
    offer_index offers;

    // @abi table result i64
    struct result {
        account_name owner;
        uint64_t roll_number;

        uint64_t primary_key() const { return account; }
        EOSLIB_SERIALIZE(result, (owner)(roll_number))
    };
    typedef eosio::multi_index<N(result), result> result_index;
    result_index results;      
    
    void sub_balance(account_name owner, asset value);
    void add_balance(account_name owner, asset value, account_name ram_payer);
    
public:
    struct transfer_args {
        account_name  from;
        account_name  to;
        asset         quantity;
        string        memo;
    };
};

asset token::get_supply(symbol_name sym)const
{
    stats statstable(_self, sym);
    const auto& st = statstable.get(sym);
    return st.supply;
}

asset token::get_balance(account_name owner, symbol_name sym)const
{
    accounts accountstable(_self, owner);
    const auto& ac = accountstable.get(sym);
    return ac.balance;
}
