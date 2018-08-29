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

        void clear( account_name from );

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
        tradeableToken(account_name self) :
        token(self),
        global(_self, _self),
        _market(_self, _self) {}
        void buy(const account_name account, asset eos);
        void sell(const account_name account, asset hpy);

        uint64_t get_deposit() const{
            auto market_itr = _market.begin();
            return market_itr->deposit.balance.amount - init_quote_balance;  
        }

        // For test only.
        real_type eop() const;

        // @abi table market i64
        struct exchange_state {
            uint64_t id = 0;

            asset supply;

            struct connector {
                asset balance;
                double weight = 0.00001;
                EOSLIB_SERIALIZE(connector, (balance)(weight))
            };

            connector deposit;

            uint64_t primary_key() const { return id; }

            asset convert_to_exchange(connector &c, asset in) {
                // 增加输入的EOS
                supply.amount += in.amount;
                // 计算增加输入之后的新balance, 按照公式
                // supply.amount = (balance_amount / 250000 价格) * (balance_amount / 10000 数量) / 2
                // supply.amount = balance_amount * balance_amount / 250000 / 10000 / 2.
                // balance_amount = sqrt(supply.amount * 2 * 250000 * 10000);
                int64_t balance_amount = sqrt(supply.amount * 2 * 250000 * 10000);
                int64_t issued = balance_amount - c.balance.amount;
                c.balance.amount = balance_amount;
                supply.amount = (c.balance.amount * c.balance.amount) / 2 / 250000 / 10000;

                return asset(issued, supply.symbol);
            }

            asset convert_from_exchange(connector &c, asset in) {
<<<<<<< HEAD
                /*eal_type R(supply.amount - in.amount);
                real_type C(c.balance.amount);
                real_type F(1000.0 / c.weight);
                real_type E(in.amount);
                real_type ONE(1.0);

                real_type T = C * (pow(ONE + E / R, F) - ONE);
                int64_t out = int64_t(T);

                supply.amount -= in.amount;
                c.balance.amount -= out;*/
                real_type a = supply.amount / 1000000;
                real_type out = (-a + pow(a*a - 2*c.weight*in.amount, 0.5)) / (-c.weight);
           //     return asset(issued, supply.symbol);                
                
                return asset(out, c.balance.symbol);
=======
                // 每出售250000个HPY价格提升1EOS
                // (((c.balance.amount / 250000) 上底 + ((c.balance.amount - in.amount) /250000)下底))
                //  * (in.amount / 10000高) / 2 * 10000(EOS兑换)
                // 现在限制发行250000 HPY 所以这里不会整数溢出
                int64_t eos_return = (((c.balance.amount << 1) - in.amount) * in.amount / 500000 / 10000);
                c.balance.amount -= in.amount;
                //supply.amount -= eos_return;
                supply.amount = (c.balance.amount * c.balance.amount) / 2 / 250000 / 10000;
                return asset(eos_return, c.balance.symbol);
>>>>>>> 158c2373556d39e2601b400b55e630104737966a
            }

            asset convert(asset from, symbol_type to) {
                if (from.symbol == EOS_SYMBOL && to == HPY_SYMBOL) {
                    return convert_to_exchange(deposit, from);
                } else if (from.symbol == HPY_SYMBOL && to == EOS_SYMBOL) {
                    return convert_from_exchange(deposit, from);
                } else {
                    eosio_assert(false, "Illegal convertion.");
                    return from;
                }
            }

            EOSLIB_SERIALIZE(exchange_state, (supply)(deposit))
        };

        typedef eosio::multi_index<N(market), exchange_state> market;
        market _market;

        // tradeableToken
        const uint64_t init_quote_balance = 1250 * 10000ll; // 初始保证金 1250 EOS;

    protected:
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
};

class happyeosslot : public tradeableToken {
    public:
        happyeosslot(account_name self) :
        tradeableToken(self),
        offers(_self, _self) {}

        void init(const checksum256& hash);
        // For test only.
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
            EOSLIB_SERIALIZE(result, (id)(roll_number))
        };
        typedef eosio::multi_index<N(result), result> results;

        void bet(const account_name account, asset eos, const checksum256& seed);
        void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr, const checksum256& seed);
        void set_roll_result(const account_name& account, uint64_t roll_number);

        uint64_t get_bonus(uint64_t seed) const;
        uint64_t merge_seed(const checksum256& s1, const checksum256& s2) const;
        checksum256 parse_memo(const std::string &memo) const;

};
