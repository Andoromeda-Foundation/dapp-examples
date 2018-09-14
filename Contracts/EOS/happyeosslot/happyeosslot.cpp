#include <eosiolib/crypto.h>
#include "happyeosslot.hpp"

#include <cstdio>

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
    eosio_assert( existing == statstable.end(), "Token with symbol already exists" );

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
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
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
    const auto& ac = accountstable.find( sym );
    if (ac == accountstable.end()) return asset(0, HPY_SYMBOL);
    else return ac->balance;
}


void token::clear( account_name from ) {
    require_auth(from);
    accounts from_acnts( _self, from );
    while (from_acnts.begin() != from_acnts.end()) {
        from_acnts.erase(from_acnts.begin());
    }
}

//uint64_t tradeableToken::get_my_balance() const{
//}

asset current_balance;

real_type tradeableToken::eop() const {
    //if (false) return 1; // For test switch.
    const auto& sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account(TOKEN_CONTRACT, _self);
    auto old_balance = eos_account.get(sym).balance;

    auto g = global.find(0);
    old_balance.amount -= g->offerBalance;

    auto deposit = get_deposit();

    if (deposit > 0 && old_balance.amount > 0) {
        return real_type(old_balance.amount) / deposit;
    } else {
        return 1;
    }
}

void tradeableToken::buy(const account_name account, asset eos) {
//    require_auth( _self ); 
    auto market_itr = _market.begin();
    int64_t delta;
    // Calculate eop
    const auto& sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account(TOKEN_CONTRACT, _self);
    auto old_balance = eos_account.get(sym).balance - eos;

    auto g = global.find(0);
    old_balance.amount -= g->offerBalance;

    //auto old_balance = current_balance - eos;

    const auto& deposit = get_deposit();
    if (deposit > 0 && old_balance.amount > 0) {
        eos.amount = eos.amount * deposit / old_balance.amount;
    }

    eosio_assert(eos.amount > 0, "Must buy with positive Eos.");

    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(eos, HPY_SYMBOL).amount;
    });
    eosio_assert(delta > 0, "must reserve a positive amount");  
    asset hpy(delta, HPY_SYMBOL);
    issue(account, hpy, "issue some new hpy");
}

// 250000  0.1
// 

// @abi action
void tradeableToken::sell(const account_name account, asset hpy) {
    require_auth(account);
    auto market_itr = _market.begin();
    int64_t delta;
    // Calculate eop
    const auto& sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account(TOKEN_CONTRACT, _self);
    auto old_balance = eos_account.get(sym).balance;

    auto g = global.find(0);
    old_balance.amount -= g->offerBalance;
    //auto old_balance = current_balance;
    const auto& deposit = get_deposit();

    _market.modify(market_itr, 0, [&](auto &es) {
        delta = es.convert(hpy, EOS_SYMBOL).amount;
    });

    delta = delta * old_balance.amount / deposit;
    eosio_assert(delta > 0, "Must burn a positive amount");
    burn(account, hpy);
    asset eos(delta, EOS_SYMBOL);
    // transfer eos

    //current_balance -= eos;
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
            g.offerBalance = 0;
        });
    } else {
        global.modify(g, 0, [&](auto &g) {
            g.hash = hash;
        });
    }
    if (_market.begin() == _market.end()) {
        _market.emplace(_self, [&](auto &m) {
            m.supply.amount = 25000ll * 10000ll; // 最初25000个HPY是沉默资金
            m.supply.symbol = HPY_SYMBOL;
            m.deposit.balance.amount = init_quote_balance; // 
            m.deposit.balance.symbol = EOS_SYMBOL;
        });
        // 这里限制发行HPY为225000 个HPY 修改上限注意修改几个整形溢出的问题。
        create(_self, asset(2250000000ll, HPY_SYMBOL));
    }
}
 void happyeosslot::bet(const account_name account, asset eos, const checksum256& seed) {

    eosio_assert(offers.begin() == offers.end(), "only one bet at one time.");

    offers.emplace(_self, [&](auto& offer) {
        offer.id = offers.available_primary_key();
        offer.owner = account;
        offer.bet = eos.amount;
        offer.seed = seed;
    });
    auto g = global.find(0);
    global.modify(g, 0, [&](auto &g) {
        g.offerBalance += eos.amount;
    });
    set_roll_result(account, 0);
}

/*
 // @abi action
void happyeosslot::onSell(account_name from, account_name to, asset hpy, std::string memo) {        
    if (to != _self) {
        transfer()
    }
    require_auth(from);
    eosio_assert(hpy.is_valid(), "Invalid token transfer");
    eosio_assert(hpy.symbol == HPY_SYMBOL, "only HPY token allowed");
    eosio_assert(hpy.amount > 0, "must sell a positive amount");
    string operation = memo.substr(0, 4);
    if (operation == "sell") {
        sell(from, hpy);
    }
//    const checksum256 seed = parse_memo(memo);
  //  bet(from, eos, seed);
}*/

 // @abi action
void happyeosslot::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) {
        return;
    }
    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");
    string operation = memo.substr(0, 3);
    if (operation == "bet") {
        const checksum256 seed = parse_memo(memo);
        bet(from, eos, seed);
    } else if (operation == "buy") {
        if (memo.size() > 7){
            if (memo.substr(4, 3) == "for") {
                memo.erase(0, 8);
                account_name t = eosio::string_to_name(memo.c_str());
                if (is_account(t)) {
                    from = t;
                }
            }
        }
        buy(from, eos);
    } else {
        action(
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
                make_tuple(_self, N(iamnecokeine), eos, std::string("Unknown happyeosslot deposit.")))
        .send();
    }
}
 // @abi action
void happyeosslot::transfer(account_name from, account_name to, asset quantity, std::string memo) {        
    if (to == _self) {
        sell(from, quantity);
    } else {
        _transfer(from, to, quantity, memo);
    }
}
 // @abi action
void happyeosslot::reveal(const checksum256 &seed, const checksum256 &hash) {
    require_auth(_self);
    assert_sha256((char *)&seed, sizeof(seed), (const checksum256 *)&global.begin()->hash);
    auto n = offers.available_primary_key();

    /*if (n > 20){
        uint64_t delta = 0;
        for (int i = n-1; i >= 5; --i) {
            auto itr = offers.find(i);
            delta += itr->bet;
            deal_with(itr, seed);
        }
        auto itr = global.find(0);
        global.modify(itr, 0, [&](auto &g) {
            g.offerBalance -= delta;
         });
        return;
    }*/
    
    for (int i = 0; i < n; ++i) {
        auto itr = offers.find(i);
        if (itr != offers.end()) {
            deal_with(itr, seed);
        }
    }
    auto itr = global.find(0);
    global.modify(itr, 0, [&](auto &g) {
        g.hash = hash;
        g.offerBalance = 0;
    });
}
const int p[8] = {   25,   50,  120, 1000, 3000, 17000, 50000, 124805};
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
        hash ^= (s1.hash[i] ^ s2.hash[31-i]) << ((i & 7) << 3);
     //   hash ^= (s1.hash[i]) << ((i & 7) << 3);
    }
    return hash;
}
 void happyeosslot::deal_with(eosio::multi_index<N(offer), offer>::const_iterator itr, const checksum256 &seed) {
    uint64_t bonus_rate = get_bonus(merge_seed(seed, itr->seed));
    uint64_t bonus = bonus_rate * itr->bet / 100;
    if (bonus > 0) {

//          static char msg[100];
  //       sprintf(msg, "Happy eos slot bonus. happyeosslot.com: %d", bonus/10000);

        action(
                permission_level{_self, N(active)},
                N(eosio.token), N(transfer),
                make_tuple(_self, itr->owner, asset(bonus, EOS_SYMBOL),
                    std::string("Happy eos slot bonus. happyeosslot.com") ))
            .send();

      /* action(
            permission_level{_self, N(active)},
            _self, N(transfer),
            make_tuple(_self, itr->owner, asset(1, HPY_SYMBOL), std::string("HPY token airdrop.")))
            .send();  */ 
        //    issue(itr->owner, asset(1, HPY_SYMBOL), "HPY token airdrop.");    
    }
    set_roll_result(itr->owner, bonus_rate);
    offers.erase(itr);
}

checksum256 happyeosslot::parse_memo(const std::string &memo) const {
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

void happyeosslot::test(const account_name account, asset eos) {
    require_auth(_self);

   /* //eosio_assert(false, "emmm");
    static char msg[10];
    sprintf(msg, "EOP: %f", float(eop()));
    eosio_assert(false, msg);
    return;*/
 
        
   /* if (global.begin() != global.end()) {
	global.erase(global.begin());
    }
    if (_market.begin() != _market.end()) {
	_market.erase(_market.begin());
    }*/
    if (offers.begin() != offers.end()) {
	offers.erase(offers.begin());
    }    
    

    /*
    stats statstable( _self, eos.symbol.name() );
    if (statstable.begin() != statstable.end()) {
	statstable.erase(statstable.begin());
    }
    accounts minako(_self, N(minakokojima));
    while (minako.begin() != minako.end()) {
        minako.erase(minako.begin());
    }
    accounts necokeine(_self, N(iamnecokeine));
    while (necokeine.begin() != necokeine.end()) {
	necokeine.erase(necokeine.begin());
    }
    accounts tmonomonomon(_self, N(tmonomonomon));
    while (tmonomonomon.begin() != tmonomonomon.end()) {
	tmonomonomon.erase(tmonomonomon.begin());
    }    */
    return ;


    const auto& sym = eosio::symbol_type(HPY_SYMBOL).name();
    //current_balance = asset(0, EOS_SYMBOL);
    current_balance = asset(10000, EOS_SYMBOL);
    buy(account, asset(10000, EOS_SYMBOL));
    eos.amount *=2;

    auto beforebuyamount1 = get_balance(account, sym).amount;

    current_balance += eos;
    buy(account, eos);
    auto delta = get_balance(account, sym).amount - beforebuyamount1;

    current_balance += asset(10000, EOS_SYMBOL);

    eosio_assert(delta > 0, "Delta should be positive.");

    //sell(account, asset(delta, HPY_SYMBOL));
    //auto afterbuysell1 = get_balance(account, sym).amount;

    //eosio_assert(beforebuyamount1 == afterbuysell1, "not equal after sell1");

    //auto beforebuyamount2 = get_balance(account, sym).amount;
    eos.amount /= 2;
    current_balance += eos;
    buy(account, eos);
    //auto dd = get_balance(account, sym).amount;
    //auto d3 = dd - beforebuyamount1;
    current_balance += eos;
    buy(account, eos);
    //auto delta2 = get_balance(account, sym).amount - dd;
    
    //eosio_assert(delta >= delta2, "Buy one and Buy two");
    //eosio_assert(delta - delta2 > 10, "not equal when buy 2 times.");
    eosio_assert(false, "Test end");
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
            }                       \
                                                                                                                                                                             \
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
MY_EOSIO_ABI(happyeosslot, (onTransfer)(transfer)(init)(sell)(reveal)(test))

// generate .abi file
// EOSIO_ABI(happyeosslot, (transfer)(init)(sell)(reveal)(test))
