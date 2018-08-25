#include <eosiolib/crypto.h>
#include "happyeosslot.hpp"

void token::_create( account_name issuer,
                     asset        maximum_supply ) {
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

void token::_burn( account_name from, asset quantity)
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before burn" );
    const auto& st = *existing;

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must burn positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert(  st.supply.amount >= quantity.amount, "quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance(from, quantity);

    //if( to != st.issuer ) {
        // SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
        /*action(
            permission_level{_self, N(active)},
            N(_self), N(transfer),
            make_tuple(st.issuer, to, quantity, memo)))
            .send();         */
  //  }   
}

void token::_issue( account_name to, asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;
    //require_auth( st.issuer );


    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    //210000000000
    //eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );

   // eosio::print("__________________");  

    if( to != st.issuer ) {
        // SEND_INLINE_ACTION( *this, _transfer, {_self, N(active)}, {_self, to, quantity, memo} );
        //eosio_assert(false, "shab");
       // eosio::print("how many: ", quantity.amount);
        action(
            permission_level{_self, N(active)},
            _self, N(transfer),
            make_tuple(_self, to, quantity, memo))
            .send();    
    }
}

void token::_transfer( account_name from,
                       account_name to,
                       asset        quantity,
                       string       memo ) {

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

const uint64_t init_quote_balance = 50 * 10000 * 10000ll; // 初始保证金 50 万 EOS。

void tradeableToken::buy(const account_name account, asset eos) {
    eosio::print("buy ");    
   //  auto global_itr = global.begin();
 //   global_itr->realBalance += eos.amount;
    auto market_itr = _market.begin();

    int64_t delta;
    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(eos, HPY_SYMBOL).amount;
    });
    eosio::print("buy ", delta);    
    eosio_assert(delta > 0, "must reserve a positive amount");  
    asset hpy(asset(delta, HPY_SYMBOL));
    _issue(account, hpy, "issue some new hpy");
}

void tradeableToken::sell(const account_name account, asset hpy) {
    auto market_itr = _market.begin();
    int64_t delta;
    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(hpy, EOS_SYMBOL).amount;
    });
    eosio_assert(delta > 0, "must burn a positive amount");    
    _burn(account, hpy);
    asset eos(asset(delta, EOS_SYMBOL));
    // transfer eos
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, account, eos, std::string("I'll be back.")))
        .send();       
}

// @abi action
void happyeosslot::create( account_name issuer,
                           asset        maximum_supply ) {
    require_auth( _self );
    _create(issuer, maximum_supply);
}

// @abi action
void happyeosslot::issue( account_name to, asset quantity, string memo )
{
    require_auth( _self );    
    _issue(to, quantity, memo);
}

// @abi action
void happyeosslot::transfer( account_name from,
                             account_name to,
                             asset        quantity,
                             string       memo ) {
    require_auth( from );
    _transfer(from, to, quantity, memo);
}

// @abi action
void happyeosslot::init(account_name self, const checksum256 &hash) {
    eosio_assert(self == _self, "only contract itself.");
    auto g = global.find(0);
    if (g == global.end()) {
        global.emplace(_self, [&](auto &g) {
            g.id = 0;
            g.hash = hash;
        });
        
        _market.emplace(_self, [&](auto &m) {
            m.supply.amount = 100000000000000ll;
            m.supply.symbol = HPY_SYMBOL;
            m.deposit.balance.amount = init_quote_balance;
            m.deposit.balance.symbol = EOS_SYMBOL;
        });
                
        create(self, asset(210000000000, HPY_SYMBOL));    
    } else {
        global.modify(g, 0, [&](auto &g) {
            g.hash = hash;
        });
    }
}

void happyeosslot::bet(const account_name account, asset bet, const checksum256& seed) {
    eosio::print("bet ", bet);
    offers.emplace(_self, [&](auto &offer) {
        offer.id = offers.available_primary_key();
        offer.owner = account;
        offer.bet = bet.amount;
        offer.seed = seed;
    });
    
    auto p = results.find(account);
    if (p == results.end()) {
        p = results.emplace(_self, [&](auto& result) {
            result.owner = account;
            result.roll_number = 0;
        });
    } else {
        results.modify(p, 0, [&](auto& result) {
            result.roll_number = 0;
        });
    }
}

uint64_t happyeosslot::get_my_balance() const{
    auto sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account(TOKEN_CONTRACT, _self);
    auto account = eos_account.get(sym);
    return account.balance.amount;
}

real_type happyeosslot::eop()const{
    auto sym = eosio::symbol_type(EOS_SYMBOL).name();
    auto balance = eosio::token(TOKEN_CONTRACT).get_balance(_self, sym);
    eosio_assert(balance.amount == get_my_balance(), "should be equal");
 //   return 1;
   // return balance.amount;
    return real_type(balance.amount) / get_deposit();
}

void happyeosslot::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");

    auto position = memo.find("buy");
    if (position != memo.npos) {
        buy(from, eos);
    } else {
        position = memo.find("bet");
        if (position != memo.npos) {
            const checksum256 seed = parse_memo(memo); 
            bet(from, eos, seed);
        } else {
            if (eos.amount >= 0) {
                buy(from, eos);
            } else {
                const checksum256 seed = parse_memo(memo); 
                bet(from, eos, seed);             
            }
        }
    }

    eosio::print("current balance: ", get_my_balance());   
    eosio::print("current deposit: ", get_deposit());         
    eosio::print("current eop: ", eop());
}

// @abi action
void happyeosslot::reveal(const account_name host, const checksum256 &seed, const checksum256 &hash) {
    require_auth(host);
    eosio_assert(host == _self, "Only happyeosslot can reveal the answer.");
    assert_sha256((char *)&seed, sizeof(seed), (const checksum256 *)&global.begin()->hash);
    auto n = offers.available_primary_key();
    for (int i = 0; i < n; ++i) {
        auto itr = offers.find(i);
        deal_with(itr, seed);
    }
    auto itr = global.find(0);
    global.modify(itr, 0, [&](auto &g) {
        g.hash = hash;
    });
}



#define MY_EOSIO_ABI(TYPE, MEMBERS)                                                                                  \
    extern "C"                                                                                                       \
    {                                                                                                                \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                \
        {                                                                                                            \
                                                                                                                     \
            auto self = receiver;                                                                                    \
            if (action == N(onerror))                                                                                \
            {                                                                                                        \
                eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
            }                                                                                                        \
            if (code == TOKEN_CONTRACT && action == N(transfer)) {                                                   \
                action = N(onTransfer);                                                                              \
            }                                                                                                        \
            if ((code == TOKEN_CONTRACT && action == N(onTransfer)) || code == self && action != N(onTransfer)) {                               \
                TYPE thiscontract(self);                                                                             \
                switch (action)                                                                                      \
                {                                                                                                    \
                    EOSIO_API(TYPE, MEMBERS)                                                                         \
                }                                                                                                    \
            }                                                                                                        \
        }                                                                                                            \
    }
// generate .wasm and .wast file
MY_EOSIO_ABI(happyeosslot, (create)(issue)(onTransfer)(transfer)(init)(sell)(reveal))

// generate .abi file
// EOSIO_ABI(happyeosslot, (create)(issue)(transfer)(init)(sell)(reveal))
