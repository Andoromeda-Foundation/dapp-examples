#include <eosiolib/crypto.h>
#include "happyeosslot.hpp"

// Token
void token::create( account_name issuer,
                     asset        maximum_supply ) {
    require_auth( _self );
    auto sym = maximum_supply.symbol;
    eosio_assert( sym.is_valid(), "Invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "Invalid supply");
    eosio_assert( maximum_supply.amount > 0, "Max-supply must be positive");

    stats statstable( _self, sym.name() );
    auto existing = statstable.find( sym.name() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}

void token::burn( account_name from, asset quantity ) {
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "Invalid symbol name" );

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

void token::issue( account_name to, asset quantity, string memo ) {
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
        // SEND_INLINE_ACTION( *this, transfer, {_self, N(active)}, {_self, to, quantity, memo} );
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

void token::add_balance( account_name owner, asset value, account_name ram_payer ) {
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

asset token::get_supply( symbol_name sym )const {
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );
    return st.supply;
}

asset token::get_balance( account_name owner, symbol_name sym )const {
    accounts accountstable( _self, owner );
    const auto& ac = accountstable.get( sym );
    return ac.balance;
}

//uint64_t tradeableToken::get_my_balance() const{
//}

real_type tradeableToken::eop() const {
    auto sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account(TOKEN_CONTRACT, _self);
    auto account = eos_account.get(sym);
    auto balance = account.balance;
    // auto balance = eosio::token(TOKEN_CONTRACT).get_balance(_self, sym);
   
    //auto sym = eosio::symbol_type(EOS_SYMBOL).name();
    auto deposit = get_deposit();
//    eosio_assert(balance.amount == get_my_balance(), "should be equal");
    if (deposit > 0) {
        return real_type(balance.amount) / get_deposit();
    } else {
        return 1;
    }
}

void tradeableToken::buy(const account_name account, asset eos) {
   auto market_itr = _market.begin();
    int64_t delta;
    eos.amount /= eop();
    eosio_assert(eos.amount > 0, "Must buy with positive Eos.");

    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(eos, HPY_SYMBOL).amount;
    });
    eosio_assert(delta > 0, "must reserve a positive amount");  
    asset hpy(delta, HPY_SYMBOL);
    issue(account, hpy, "issue some new hpy");
}

// @abi action
void tradeableToken::sell(const account_name account, asset hpy) {
    require_auth(account);
    auto market_itr = _market.begin();
    int64_t delta;
    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(hpy, EOS_SYMBOL).amount;
    });
    delta *= eop();
    eosio_assert(delta > 0, "Must burn a positive amount");    
    burn(account, hpy);
    asset eos(delta, EOS_SYMBOL);
    // transfer eos
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, account, eos, std::string("Sell happyeosslot.com share HPY.")))
        .send();
}
// Happyeosslot

// @abi action
void happyeosslot::init(const checksum256 &hash) {
    require_auth( _self );
    auto g = global.find(0);
    if (g == global.end()) {
        global.emplace(_self, [&](auto &g) {
            g.id = 0;
            g.hash = hash;
        });
    } else {
        global.modify(g, 0, [&](auto &g) {
            g.hash = hash;
        });
    }
    if (_market.begin() == _market.end()) {
        _market.emplace(_self, [&](auto &m) {
            m.supply.amount = 2000000000000ll;
            m.supply.symbol = HPY_SYMBOL;
            m.deposit.balance.amount = init_quote_balance;
            m.deposit.balance.symbol = EOS_SYMBOL;
        });                
        create(_self, asset(210000000000ll, HPY_SYMBOL));
    }
}

void happyeosslot::bet(const account_name account, asset eos, const checksum256& seed) {
    offers.emplace(_self, [&](auto& offer) {
        offer.id = offers.available_primary_key();
        offer.owner = account;
        offer.bet = eos.amount;
        offer.seed = seed;
    });
    
    set_roll_result(account, 0);
}

// @abi action
void happyeosslot::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(eos.is_valid(), "invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");

    string operation = memo.substr(0, 3);
    if (operation == "bet" || ((operation != "buy") && (eos.amount >= 100000))) {
        const checksum256 seed = parse_memo(memo);
        bet(from, eos, seed);
    } else {
        buy(from, eos);
    }
//    const checksum256 seed = parse_memo(memo);
  //  bet(from, eos, seed);
}

// @abi action
void happyeosslot::transfer(account_name from, account_name to, asset quantity, std::string memo) {        
    _transfer(from, to, quantity, memo);
}

// @abi action
void happyeosslot::reveal(const checksum256 &seed, const checksum256 &hash) {
    require_auth(_self);
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

const int p[8] = {   25,   50,  120, 1000, 4000, 20000, 50000, 24805};
const int b[8] = {10000, 5000, 2000, 1000,  500,   200,    10,     1};

uint64_t happyeosslot::get_bonus(uint64_t seed) const {
    seed %= 100000;
    int i = 0;
    while (seed >= p[i]) {
        seed -= p[i];
        ++i;
    }
    return b[i];
}

uint64_t happyeosslot::merge_seed(const checksum256 &s1, const checksum256 &s2) const {
    uint64_t hash = 0, x;
    for (int i = 0; i < 32; ++i) {
        hash ^= (s1.hash[i] ^ s2.hash[i]) << ((i & 7) << 3);
    }
    return hash;
}

void happyeosslot::deal_with(eosio::multi_index<N(offer), offer>::const_iterator itr, const checksum256 &seed) {
    uint64_t bonus_rate = get_bonus(merge_seed(seed, itr->seed));
    uint64_t bonus = bonus_rate * itr->bet / 100;
    if (bonus > 0) {
        action(
                permission_level{_self, N(active)},
                N(eosio.token), N(transfer),
                make_tuple(_self, itr->owner, asset(bonus, EOS_SYMBOL),
                    std::string("Happy eos slot bonus. happyeosslot.com")))
            .send();
    }
    set_roll_result(itr->owner, bonus_rate);
    offers.erase(itr);
}

checksum256 happyeosslot::parse_memo(const std::string &memo) const { // to bo refine.
    checksum256 checksum;
    memset(&checksum, 0, sizeof(checksum256));
    for (int i = 0; i < memo.length(); i++) {
        checksum.hash[i & 31] ^= memo[i];
    }
    return checksum;
}

void happyeosslot::set_roll_result(const account_name& account, uint64_t roll_number) {
    results res_table(_self, account);

    auto res = res_table.begin();

    if( res == res_table.end() ) {
        res_table.emplace( _self /* ram payer*/, [&]( auto& res ){
            res.id = 0;
            res.roll_number = roll_number;
        });
    } else {
        res_table.modify( res, 0 /* ram payer */, [&]( auto& res ) {
            res.roll_number = roll_number;
        });
    }
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
                }                                                                                                     \
            }                                                                                                        \
        }                                                                                                            \
    }
// generate .wasm and .wast file
MY_EOSIO_ABI(happyeosslot, (onTransfer)(transfer)(init)(sell)(reveal))

// generate .abi file
// EOSIO_ABI(happyeosslot, (transfer)(init)(sell)(reveal))
