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

class token : public contract {
    public:
        token( account_name self ):contract(self){}

        void create( account_name issuer,
                     asset        maximum_supply);

        void issue( account_name to, asset quantity, string memo );
        void burn( account_name from, asset quantity );

        void _transfer( account_name from,
                       account_name to,
                       asset        quantity,
                       string       memo );


        inline asset get_supply( symbol_name sym )const;

        inline asset get_balance( account_name owner, symbol_name sym )const;

    private:
        void sub_balance( account_name owner, asset value );
        void add_balance( account_name owner, asset value, account_name ram_payer );

    public:  
        struct account {
            asset    balance;
            uint64_t primary_key() const { return balance.symbol.name(); }
        };
        struct currency_stats {
            asset          supply;
            asset          max_supply;
            account_name   issuer;
            uint64_t primary_key() const { return supply.symbol.name(); }
        };
        typedef eosio::multi_index<N(accounts), account> accounts;
        typedef eosio::multi_index<N(stat), currency_stats> stats;
        struct transfer_args {
            account_name  from;
            account_name  to;
            asset         quantity;
            string        memo;
        };
};

class tradeableToken : public token {
    public:    
        tradeableToken(account_name self) : token(self), _market(_self, _self) {}
        void buy(const account_name account, asset eos);
        void sell(const account_name account, asset hpy);

        uint64_t get_deposit() const{
            auto market_itr = _market.begin();
            return market_itr->deposit.balance.amount - init_quote_balance;  
        }

//        real_type raw_price() const{
//            auto market_itr = _market.begin();
//            return market_itr->get_price(); 
//        }

        //uint64_t get_my_balance() const;
        real_type eop(asset current_deposit) const;

        // @abi table market i64
        struct exchange_state {
            uint64_t id = 0;

            asset supply;

            struct connector {
                asset balance;
                double weight = .5;
                EOSLIB_SERIALIZE(connector, (balance)(weight))
            };

            connector deposit;

            uint64_t primary_key() const { return id; }

            asset convert_to_exchange(connector &c, asset in) {
                real_type R(supply.amount);
                real_type C(c.balance.amount + in.amount);
                real_type F(c.weight / 1000.0);
                real_type T(in.amount);
                real_type ONE(1.0);

                real_type E = -R * (ONE - pow(ONE + T / C, F));
                int64_t issued = int64_t(E);

                supply.amount += issued;
                c.balance.amount += in.amount;

                return asset(issued, supply.symbol);
            }

            asset convert_from_exchange(connector &c, asset in) {
                real_type R(supply.amount - in.amount);
                real_type C(c.balance.amount);
                real_type F(1000.0 / c.weight);
                real_type E(in.amount);
                real_type ONE(1.0);

                real_type T = C * (pow(ONE + E / R, F) - ONE);
                int64_t out = int64_t(T);

                supply.amount -= in.amount;
                c.balance.amount -= out;

                return asset(out, c.balance.symbol);
            }

            asset convert(asset from, symbol_type to) {
                if (from.symbol == EOS_SYMBOL && to == HPY_SYMBOL) {
                    return convert_to_exchange(deposit, from);
                } else if (from.symbol == HPY_SYMBOL && to == EOS_SYMBOL) {
                    return convert_from_exchange(deposit, from);
                } else {
                    eosio_assert(false, "illegal convertion.");
                    return from;
                }
            }

            EOSLIB_SERIALIZE(exchange_state, (supply)(deposit))
        };

        typedef eosio::multi_index<N(market), exchange_state> market;
        market _market;

        // tradeableToken
        const uint64_t init_quote_balance = 1 * 10000 * 10000ll; // 初始保证金 1 万 EOS。;  
};

class happyeosslot : public tradeableToken {
    public:
        happyeosslot(account_name self) : tradeableToken(self),
        global(_self, _self),
        offers(_self, _self) {}

        void init(const checksum256& hash);
        void test(const account_name account, asset eos);
        
        // EOS transfer event.
        void onTransfer(account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo);

        void transfer( account_name from,
                       account_name to,
                       asset        quantity,
                       string       memo );                        

        void reveal( const checksum256 &seed, const checksum256 &hash);
//        real_type price() const{
//            return raw_price() * eop();
//        }

//        void apply(account_name contract, account_name act);

//        uint64_t get_roll_result(const account_name& account) const;

    private:
        // @abi table global i64
        struct global {
            uint64_t id = 0;
            checksum256 hash; // hash of the game seed, 0 when idle.
            uint64_t realBalance;

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
            uint64_t id;            
            uint64_t roll_number;
            uint64_t primary_key() const { return id; }
        };
        typedef eosio::multi_index<N(result), result> results;

        void bet(const account_name account, asset eos, const checksum256& seed);
        void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr, const checksum256& seed);
        void set_roll_result(const account_name& account, uint64_t roll_number);

        uint64_t get_bonus(uint64_t seed) const;
        uint64_t merge_seed(const checksum256& s1, const checksum256& s2) const;
        checksum256 parse_memo(const std::string &memo) const;

};
