/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

typedef double real_type;

using std::string;
using eosio::symbol_name;
using eosio::asset;
using eosio::symbol_type;
using eosio::contract;
using eosio::permission_level;
using eosio::action;

class eosslgbackup : public contract {
    public:
    eosslgbackup( account_name self ):contract(self),
        players(_self, _self) {}
        
    void test(const account_name account, asset eos);
    void buy(const account_name account, asset eos);
    void sell(const account_name account, asset eos);
    void take(const account_name from, const account_name to, asset eos);
    void bonus(const account_name account, asset eos);
    void burn(const account_name account, asset eos);

    void onTransfer(account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo);    

    // @abi table card i64
    struct player {
        account_name account;
        asset balance;
        uint64_t primary_key() const { return account; }
        EOSLIB_SERIALIZE(player, (account)(balance))
    };
    typedef eosio::multi_index<N(player), player> player_index;
    player_index players;    

    private:
    void _add_price(const account_name account, asset eos);
    void _sub_price(const account_name account, asset eos);
};
