/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

#include <string>

namespace eosiosystem {
      class system_contract;
}

namespace eosio {

      using std::string;

      class token : public contract {
      public:
            token( account_name self ):contract(self){}
            // @abi action
            void create( account_name issuer,
                        asset        maximum_supply);
            // @abi action                      
            void issue( account_name to, asset quantity, string memo );
            // @abi action
            void transfer( account_name from,
                              account_name to,
                              asset        quantity,
                              string       memo );
            
            
            inline asset get_supply( symbol_name sym )const;
            
            inline asset get_balance( account_name owner, symbol_name sym )const;

            struct account {
                  asset    balance;

                  uint64_t primary_key()const { return balance.symbol.name(); }
            };

            struct currency_stats {
                  asset          supply;
                  asset          max_supply;
                  account_name   issuer;

                  uint64_t primary_key()const { return supply.symbol.name(); }
            };

            typedef eosio::multi_index<N(accounts), account> accounts;
            typedef eosio::multi_index<N(stat), currency_stats> stats;

            void sub_balance( account_name owner, asset value );
            void add_balance( account_name owner, asset value, account_name ram_payer );

      public:
            struct transfer_args {
                  account_name  from;
                  account_name  to;
                  asset         quantity;
                  string        memo;
            };
      };

      asset token::get_supply( symbol_name sym )const
      {
            stats statstable( _self, sym );
            const auto& st = statstable.get( sym );
            return st.supply;
      }

      asset token::get_balance( account_name owner, symbol_name sym )const
      {
            accounts accountstable( _self, owner );
            const auto& ac = accountstable.get( sym );
            return ac.balance;
      }

      void token::create( account_name issuer,
                        asset        maximum_supply )
      {
            require_auth( _self );

            auto sym = maximum_supply.symbol;
            eosio_assert( sym.is_valid(), "invalid symbol name" );
            eosio_assert( maximum_supply.is_valid(), "invalid supply");
            eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");

            stats statstable( _self, sym.name() );
            auto existing = statstable.find( sym.name() );
            eosio_assert( existing == statstable.end(), "token with symbol already exists" );

            statstable.emplace( _self, [&]( auto& s ) {
                  s.supply.symbol = maximum_supply.symbol;
                  s.max_supply    = maximum_supply;
                  s.issuer        = issuer;
            });
      }   

      void token::issue( account_name to, asset quantity, string memo )
      {
            auto sym = quantity.symbol;
            eosio_assert( sym.is_valid(), "invalid symbol name" );
            eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

            auto sym_name = sym.name();
            stats statstable( _self, sym_name );
            auto existing = statstable.find( sym_name );
            eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
            const auto& st = *existing;
// #
            // require_auth( st.issuer );
            eosio_assert( quantity.is_valid(), "invalid quantity" );
            eosio_assert( quantity.amount > 0, "must issue positive quantity" );

            eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
            eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

            statstable.modify( st, 0, [&]( auto& s ) {
                  s.supply += quantity;
            });

            add_balance( st.issuer, quantity, st.issuer );

            if( to != st.issuer ) {
                  SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
            }
      }

      void token::transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo )
      {
            eosio_assert( from != to, "cannot transfer to self" );
            require_auth( from );
            eosio_assert( is_account( to ), "to account does not exist");
            auto sym = quantity.symbol.name();
            stats statstable( _self, sym );
            const auto& st = statstable.get( sym );

            require_recipient( from );
            require_recipient( to );

            eosio_assert( quantity.is_valid(), "invalid quantity" );
            eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
            eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
            eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );


            sub_balance( from, quantity );
            add_balance( to, quantity, from );
      }

      void token::sub_balance( account_name owner, asset value ) {
            accounts from_acnts( _self, owner );

            const auto& from = from_acnts.get( value.symbol.name(), "no balance object found" );
            eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );


            if( from.balance.amount == value.amount ) {
                  from_acnts.erase( from );
            } else {
                  from_acnts.modify( from, owner, [&]( auto& a ) {
                  a.balance -= value;
                  });
            }
      }

      void token::add_balance( account_name owner, asset value, account_name ram_payer )
      {
            accounts to_acnts( _self, owner );
            auto to = to_acnts.find( value.symbol.name() );
            if( to == to_acnts.end() ) {
                  to_acnts.emplace( ram_payer, [&]( auto& a ){
                  a.balance = value;
                  });
            } else {
                  to_acnts.modify( to, 0, [&]( auto& a ) {
                  a.balance += value;
                  });
            }
      }      

} /// namespace eosio