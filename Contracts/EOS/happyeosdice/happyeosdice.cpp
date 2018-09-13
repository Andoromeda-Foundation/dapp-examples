#include <eosiolib/crypto.h>
#include <cstdio>
#include "happyeosdice.hpp"

void happyeosdice::init(const checksum256 &hash) {
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

void happyeosdice::test(const account_name account, asset eos) {
    require_auth(_self);
}

void happyeosdice::buy(const account_name account, asset eos) {
//    require_auth( _self ); 
    auto market_itr = _market.begin();
    int64_t delta;
    // Calculate eop
    const auto& sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account(N(eosio.token), _self);
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

void happyeosdice::sell(const account_name account, asset hpy) {
    require_auth(account);
    auto market_itr = _market.begin();
    int64_t delta;
    // Calculate eop
    const auto& sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account(N(eosio.token), _self);
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

    stats statstable( _self, sym );
    auto existing = statstable.find( sym );
    eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto& st = *existing;

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply -= hpy;
    });

    //token::sub_balance(account, hpy);

    asset eos(delta, EOS_SYMBOL);
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
                make_tuple(_self, account, eos, std::string("Sell happyeosslot.com share HPY.")))
        .send();
}


void happyeosdice::send_referal_bonus(const account_name referal, asset eos) {
    if (!is_account(referal)) return;

    
            eos.amount /= 200; // 0.5%

            if (eos.amount == 0) return;

            /* old_balance 
            if（eos_account.get(sym)
            = eos_account.get(sym).balance.amount;*/

            auto ref = eosio::name{referal}.to_string();

            action(
                permission_level{_self, N(active)},
                N(eosio.token), N(transfer),
                make_tuple(_self, N(happyeosslot), asset(eos.amount, EOS_SYMBOL),
                    std::string("buy for " + ref)))
            .send();

/*
            const auto& sym = eosio::symbol_type(HPY_SYMBOL).name();
            accounts eos_account(N(happyeosslot), _self);
            uint64_t delta = eos_account.get(sym).balance.amount;

            action(
                permission_level{_self, N(active)},
                N(happyeosslot), N(transfer),
                make_tuple(_self, referal, asset(delta, HPY_SYMBOL),
                    std::string("Referal bonus.")  ))
            .send();*/

            /*

            action(
                permission_level{_self, N(active)},
                N(happyeosslot), N(transfer),
                make_tuple(_self, referal, asset(delta, HPY_SYMBOL),
                    std::string("Referal bonus.")  ))
            .send(); */
}




void happyeosdice::bet(const account_name account, const account_name referal, asset eos, const checksum256& seed, const uint64_t bet_number) {
//  require_auth( _self );
    // eosio_assert(bet_number >= 0, "Bet number should bigger or equal to 0."); always true.
    eosio_assert((2 <= bet_number && bet_number <= 97) || (102 <= bet_number && bet_number <= 197) ,  "Bet number should smaller than 100.");
    send_referal_bonus(referal, eos);

    eosio_assert(offers.begin() == offers.end(), "only one bet at one time.");

    offers.emplace(_self, [&](auto& offer) {
        offer.id = offers.available_primary_key();
        offer.owner = account;
        offer.under = bet_number;
        offer.bet = eos.amount;
        offer.seed = seed;
    });
    auto g = global.find(0);
    global.modify(g, 0, [&](auto &g) {
        g.offerBalance += eos.amount;
    });
    set_roll_result(account, 256);

    const st_bet _bet{.player = from,
                      .referrer = referrer,
                      .amount = quantity,
                      .roll_above = 0,                      
                      .roll_under = roll_under,
                      .server_hash = g->hash,
                      .client_seed = seed,
                      };    
    action(permission_level{_self, N(active)},
        _self, N(bet_receipt), _bet)
    .send();                      
}

void happyeosdice::bet_receipt(const st_bet& bet) {
    require_auth(_self);
}

void happyeosdice::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) {
        return;
    }

    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");
    stringSplitter stream(memo);

    string operation;
    // bet 50 safdsa iam
    stream.get_string(&operation);

/*
    static char msg[30];
    sprintf(msg, "%s", operation.c_str());
    eosio_assert(false, msg);
    return;     */  

    if (operation == "bet" ) {        
        uint64_t under;
        stream.get_uint(&under);
        string seed_string;
        if (!stream.eof()) {
            stream.get_string(&seed_string);
        }

        const checksum256 seed = parse_memo(seed_string);
        string referal_string("iamnecokeine");
        if (!stream.eof()) {
            stream.get_string(&referal_string);
        }
        account_name referal = eosio::string_to_name(referal_string.c_str());

        bet(from, referal, eos, seed, under);
    } else if (operation == "buy") {
        buy(from, eos);
    } else {
    }
}

 // @abi action
void happyeosdice::onSell(account_name from, account_name to, asset hpy, std::string memo) {        
    if (to != _self) return;

    require_auth(from);
    eosio_assert(hpy.is_valid(), "Invalid token transfer");
    eosio_assert(hpy.symbol == HPY_SYMBOL, "only HPY token allowed");
    eosio_assert(hpy.amount > 0, "must sell a positive amount");
    string operation = memo.substr(0, 4);
    if (operation == "sell") {
        sell(from, hpy);
    }
}

void happyeosdice::reveal(const checksum256 &seed, const checksum256 &hash) {
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
        g.offerBalance = 0;
    });
}

void happyeosdice::create(account_name issuer, asset maximum_supply) {        
    create(issuer, maximum_supply);
}

void happyeosdice::issue(account_name to, asset quantity, std::string memo) {        
    issue(to, quantity, memo);
}

void happyeosdice::transfer(account_name from, account_name to, asset quantity, std::string memo) {        
    if (to == _self) {
        sell(from, quantity);
    } else {
        transfer(from, to, quantity, memo);
    }
}

uint64_t happyeosdice::get_bonus(uint64_t seed) const {
    seed %= 100;
    return seed;
}

uint64_t happyeosdice::merge_seed(const checksum256 &s1, const checksum256 &s2) const {
    uint64_t hash = 0, x;
    for (int i = 0; i < 32; ++i) {
        hash ^= (s1.hash[i] ^ s2.hash[31-i]) << ((i & 7) << 3);
     //   hash ^= (s1.hash[i]) << ((i & 7) << 3);
    }
    return hash;
}

 void happyeosdice::deal_with(eosio::multi_index<N(offer), offer>::const_iterator itr, const checksum256 &seed) {
    uint64_t bonus_rate = get_bonus(merge_seed(seed, itr->seed));
//    uint64_t bonus = bonus_rate * itr->bet / 100;

    if ((itr->under < bonus_rate) || (itr->under > bonus_rate + 100)) {
        int return_rate;

        if (itr->under < 100) { // 猜大
            return_rate = (99 - itr->under); // 最小猜0 itr->under = 0 赔率...., 最大猜 99 itr->under = 99 赔率98倍。
        } else { // 猜小
            return_rate = (itr->under - (100) ); // 最小猜0 itr->under = 100 赔率98倍. 最大猜 99 itr->under = 199, 赔率...
        }  

        eosio::transaction tx;

        tx.actions.emplace_back(
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, itr->owner, asset(itr->bet * 98 / return_rate , EOS_SYMBOL),
                std::string("happy eos dice bonus. The result is: ") + int_to_string(bonus_rate) + std::string(" happyeosslot.com") )
            );
            /*
        action(
                permission_level{_self, N(active)},
                N(eosio.token), N(transfer),
                make_tuple(_self, itr->owner, asset(itr->bet * 98 / return_rate , EOS_SYMBOL),
                    std::string("happy eos dice bonus. The result is: ") + int_to_string(bonus_rate) + std::string(" happyeosslot.com") ))
            .send();
    */
        tx.delay_sec = 10;
        tx.send((uint64_t)seed, _self); // need set sender_id
                  
    } else {
        if (itr->bet / 200 > 0) {        
            auto tar = eosio::name{itr->owner}.to_string();

            eosio::transaction tx;

            tx.actions.emplace_back(
                permission_level{_self, N(active)},
                N(eosio.token), N(transfer),
                make_tuple(_self, N(happyeosslot), asset(itr->bet / 200 , EOS_SYMBOL),
                    std::string("buy for " + tar))
            );
            
            tx.delay_sec = 10;
            // Sending a deferred transaction requires both a uint64_t sender_id to reference the transaction,
            // and an account_name payer which will provide the RAM to store our delayed transaction until it’s executed.
            tx.send((uint64_t)seed, _self); // need set sender_id
        }        
    }
//          static char msg[100];
//       sprintf(msg, "Happy eos slot bonus. happyeosdice.com: %d", bonus/10000); 
    set_roll_result(itr->owner, bonus_rate);
    offers.erase(itr);
}

checksum256 happyeosdice::parse_memo(const std::string &memo) const {
    checksum256 checksum;
    memset(&checksum, 0, sizeof(checksum256));
    for (int i = 0; i < memo.length(); i++) {
        checksum.hash[i & 31] ^= memo[i];
    }
    return checksum;
}

void happyeosdice::set_roll_result(const account_name account, uint64_t roll_number) {
    results res_table(_self, account);

    auto res = res_table.begin();

    if( res == res_table.end() ) {
        res_table.emplace( _self , [&]( auto& res ){
            res.id = 0;
            res.roll_number = roll_number;
        });
    } else {
        res_table.modify( res, 0 , [&]( auto& res ) {
            res.roll_number = roll_number;
        });
    }
}

// generate .abi file

//EOSIO_ABI( eosio::token, (create)(issue)(transfer) )
//EOSIO_ABI(happyeosdice, (init)(test)(reveal))