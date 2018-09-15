#include <eosiolib/crypto.h>
#include <cstdio>
#include "happyeosdice.hpp"

void happyeosdice::init(const checksum256 &hash) {
    require_auth( _self );
    
    auto g = global.find(0);
    if (g == global.end()) {        
        global.emplace(_self, [&](auto &g) {
            g.id = 0;
            g.defer_id = 0;
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
            m.supply.amount = 25000ll * 10000ll; // 最初 25000 个 DMT 是沉默资金
            m.supply.symbol = DMT_SYMBOL;
            m.deposit.balance.amount = init_quote_balance; // 
            m.deposit.balance.symbol = EOS_SYMBOL;
        });
        // 这里限制发行 DMT 为 225000 个 DMT 修改上限注意修改几个整形溢出的问题。
        create(_self, asset(2250000000ll, DMT_SYMBOL));
    }
}

void happyeosdice::test(const account_name account, asset eos) {
    require_auth(_self);    

    while (global.begin() != global.end()) {
	    global.erase(global.begin());
    }
    
    while (_market.begin() != _market.end()) {
	    _market.erase(_market.begin());
    } 
    while(offers.begin() != offers.end()) {
	    offers.erase(offers.begin());
    }    
}

real_type happyeosdice::grief_ratio() const {
    //if (false) return 1; // For test switch.
    const auto& sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account(N(eosio.token), _self);
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

void happyeosdice::buy(const account_name account, asset eos) {
//    require_auth( _self );     

  //  eosio_assert(1 == 0, "mufrsafast reserve a positive amount"); 
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
        delta = es.convert(eos, DMT_SYMBOL).amount;
    });
    eosio_assert(delta > 0, "must reserve a positive amount!");  
    asset dmt(delta, DMT_SYMBOL);
    issue(account, dmt, "issue some new dmt");

    struct rec_buy {
        account_name buyer;
        asset eos;
        asset dmt;
        uint32_t average_price;
    };

    /*
    const rec_buy _buy{.buyer = account,
                       .eos = eos,
                       .dmt = dmt,
                       .average_price = uint32_t(eos.amount / dmt.amount)
                      };   

    action(permission_level{_self, N(active)},
        _self, N(buyreceipt), _buy)
    .send(); */
}

void happyeosdice::sell(const account_name account, asset dmt) {
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
        delta = es.convert(dmt, EOS_SYMBOL).amount;
    });

    delta = delta * old_balance.amount / deposit;
    eosio_assert(delta > 0, "Must burn a positive amount");

    stats statstable( _self, sym );
    auto existing = statstable.find( sym );
    eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto& st = *existing;

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply -= dmt;
    });

    sub_balance(account, dmt);

    asset eos(delta, EOS_SYMBOL);
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
                make_tuple(_self, account, eos, std::string("Sell happyeosslot.com share DMT.")))
        .send();
}

void happyeosdice::send_referal_bonus(const account_name referal, asset eos) {
    if (!is_account(referal)) return;    
    eos.amount /= 200; // 0.5%
    if (eos.amount == 0) return;
}

void happyeosdice::bet(const account_name account, const account_name referal, asset eos, const checksum256& seed, const uint64_t bet_number) {
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

    uint8_t roll_above = -1, roll_under = -1;
    if (bet_number < 100){
        roll_above = bet_number;
    } else {
        roll_under = bet_number - 100;
    }

    const rec_bet _bet{.player = account,
                       .referrer = referal,
                       .amount = eos,
                       .roll_above = roll_above,                      
                       .roll_under = roll_under,
                       .server_hash = g->hash,
                       .client_seed = seed
                      };   
    action(permission_level{_self, N(active)},
        _self, N(betreceipt), _bet)
    .send();                   
}

void happyeosdice::betreceipt(const rec_bet& bet) {
    require_auth(_self);
}
void happyeosdice::receipt(const rec_reveal& bet) {
    require_auth(_self);
}
void happyeosdice::buyreceipt(const rec_buy& bet) {
    require_auth(_self);
}
void happyeosdice::sellreceipt(const rec_sell& bet) {
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
    stream.get_string(&operation);

    if (operation == "bet") {        
        uint64_t under;
        stream.get_uint(&under);
        string seed_string;
        if (!stream.eof()) {
            stream.get_string(&seed_string);
        }
        const checksum256 seed = parse_memo(seed_string);
        string referal_string("minakokojima");
        if (!stream.eof()) {
            stream.get_string(&referal_string);
        }
        account_name referal = eosio::string_to_name(referal_string.c_str());
        bet(from, referal, eos, seed, under);
    } else if (operation == "buy") {
        
        if (memo.size() > 7) {
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
                make_tuple(_self, N(minakokojima), eos, std::string("Unknown happyeosslot deposit."))
        ).send();
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

void happyeosdice::transfer(account_name from, account_name to, asset quantity, std::string memo) {        
    if (to == _self) {
        sell(from, quantity);
    } else {
        token::transfer(from, to, quantity, memo);
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
                    std::string("... happy eos dice bonus. The result is: ") + int_to_string(bonus_rate) + std::string(" happyeosslot.com") ))
            .send();       
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

    /*
    const rec_reveal _reveal{
        .player = itr->owner
    };   
    action(permission_level{_self, N(active)},
        _self, N(receipt), _reveal)
    .send();     */
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