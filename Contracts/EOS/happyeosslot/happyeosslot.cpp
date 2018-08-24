#include <eosiolib/crypto.h>

#include "happyeosslot.hpp"

// @abi action
void token::_create( account_name issuer,
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

    /*if( to != st.issuer ) {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
    } */   
}

// @abi action
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

    require_auth( st.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );

    if( to != st.issuer ) {
       SEND_INLINE_ACTION( *this, _transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
    }
}

// @abi action
void token::_transfer( account_name from,
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

void tradeableToken::buy(const account_name account, asset eos) {
    auto market_itr = _market.begin();
    int64_t delta;
    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(eos, HPY_SYMBOL).amount;
    });
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
                    asset        maximum_supply )
{
    _create(issuer, maximum_supply);
}

// @abi action
void happyeosslot::issue( account_name to, asset quantity, string memo )
{
    _issue(to, quantity, memo);
}

// @abi action
void happyeosslot::transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       memo )
{
    eosio::print("Transfer ");        
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
        create(self, asset(21000000.0000, HPY_SYMBOL));    
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

void happyeosslot::ontransfer(account_name from, account_name to, asset eos, std::string memo) {
    eosio::print("on Transfer ");    
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");

    if (eos.amount >= 10) {
        buy(from, eos);
    } else {
        const checksum256 seed = parse_memo(memo);        
        bet(from, eos, seed);
    }
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
        {                                              \
                                                              \
            auto self = receiver;                                                                                    \
            if (action == N(onerror))                                                                                \
            {                                                                                                        \
                eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
            }                                                                                                        \
            if (code == TOKEN_CONTRACT) { \
                                                                   \
                action = N(ontransfer)                                                                           \
            } \
            if ((code == TOKEN_CONTRACT && action == N(ontransfer)) || code == self) {     \
                                             \
                          \
                TYPE thiscontract(self);                                                                             \
                switch (action)                                                                                      \
                {                                                                                                    \
                    EOSIO_API(TYPE, MEMBERS)                                                                         \
                }                                                                                                    \
            }                                                                                                        \
        }                                                                                                            \
    }
// generate .wasm and .wast file
// MY_EOSIO_ABI(happyeosslot, (create)(issue)(transfer)(ontransfer)(init)(sell)(reveal))

// generate .abi file
EOSIO_ABI(happyeosslot, (create)(issue)(transfer)(ontransfer)(init)(sell)(reveal))

