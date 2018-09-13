#include "dicemaster11.hpp"
/*
void fairdicegame::reveal(const uint64_t& id, const checksum256& seed) {
    require_auth(REVEALER);
    st_bet bet = find_or_error(id);
    assert_seed(seed, bet.seed_hash);

    uint8_t random_roll = compute_random_roll(seed, bet.user_seed_hash);
    asset payout = asset(0, EOS_SYMBOL);
    if (random_roll < bet.roll_under) {
        payout = compute_payout(bet.roll_under, bet.amount);
        action(permission_level{_self, N(active)},
               N(eosio.token),
               N(transfer),
               make_tuple(_self, bet.player, payout, winner_memo(bet)))
            .send();
    }
    unlock(bet.amount);
    if (bet.referrer != _self) {
        // defer trx, no need to rely heavily
        transaction trx;
        trx.actions.emplace_back(permission_level{_self, N(active)},
                                 N(eosio.token),
                                 N(transfer),
                                 make_tuple(
                                     _self,
                                     bet.referrer,
                                     compute_referrer_reward(bet),
                                     referrer_memo(bet)));

        trx.send(bet.id, _self, false);
    }
    remove(bet);
    st_result result{.bet_id = bet.id,
                     .player = bet.player,
                     .referrer = bet.referrer,
                     .amount = bet.amount,
                     .roll_under = bet.roll_under,
                     .random_roll = random_roll,
                     .seed = seed,
                     .seed_hash = bet.seed_hash,
                     .user_seed_hash = bet.user_seed_hash,
                     .payout = payout};
    action(permission_level{_self, N(active)},
           LOG,
           N(result),
           result)
        .send();
}

void fairdicegame::transfer(const account_name& from,
                            const account_name& to,
                            const asset& quantity,
                            const string& memo) {
    if (from == _self || to != _self) {
        return;
    }

    uint8_t roll_under;
    checksum256 seed_hash;
    checksum160 user_seed_hash;
    uint64_t expiration;
    account_name referrer;
    signature sig;

    parse_memo(memo, &roll_under, &seed_hash, &user_seed_hash, &expiration, &referrer, &sig);

    //check quantity
    assert_quantity(quantity);

    //check roll_under
    assert_roll_under(roll_under, quantity);

    //check seed hash && expiration
    assert_hash(seed_hash, expiration);

    //check referrer
    eosio_assert(referrer != from, "referrer can not be self");

    //check signature
    assert_signature(roll_under, seed_hash, expiration, referrer, sig);

    const st_bet _bet{.id = next_id(),
                      .player = from,
                      .referrer = referrer,
                      .amount = quantity,
                      .roll_under = roll_under,
                      .seed_hash = seed_hash,
                      .user_seed_hash = user_seed_hash,
                      .created_at = now()};
    save(_bet);
    lock(quantity);
    action(permission_level{_self, N(active)},
           _self,
           N(receipt),
           _bet)
        .send();
}

void fairdicegame::receipt(const st_bet& bet) {
    require_auth(_self);
}
*/

// old

void dicemaster11::init(const checksum256 &hash) {
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
}

void dicemaster11::send_referal_bonus(const account_name referal, asset eos) {
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




void dicemaster11::bet(const account_name account, const account_name referal, asset eos, const checksum256& seed, const uint64_t bet_number) {
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
}

uint64_t string_to_int(string s) {
    uint64_t z = 0;
    for (int i=0;i<s.size();++i) {
        z += s[i] - '0';
        z *= 10;
    }
    return z;
}

 
void dicemaster11::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
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
        //buy(from, eos);
    } else {
    }
}

 
void dicemaster11::reveal(const checksum256 &seed, const checksum256 &hash) {
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

uint64_t dicemaster11::get_bonus(uint64_t seed) const {
    seed %= 100;
    return seed;
}

uint64_t dicemaster11::merge_seed(const checksum256 &s1, const checksum256 &s2) const {
    uint64_t hash = 0, x;
    for (int i = 0; i < 32; ++i) {
        hash ^= (s1.hash[i] ^ s2.hash[31-i]) << ((i & 7) << 3);
     //   hash ^= (s1.hash[i]) << ((i & 7) << 3);
    }
    return hash;
}

string int_to_string(uint64_t t) {
    if (t == 0) return "0";
    string z;
    while (t != 0) {
        z += char('0' + (t % 10));  
        t /= 10;
    }
    reverse(z.begin(), z.end());
    return z;
}

//bet 50 ludufutemp minakokojima

 void dicemaster11::deal_with(eosio::multi_index<N(offer), offer>::const_iterator itr, const checksum256 &seed) {
    uint64_t bonus_rate = get_bonus(merge_seed(seed, itr->seed));
//    uint64_t bonus = bonus_rate * itr->bet / 100;

    if ((itr->under < bonus_rate) || (itr->under > bonus_rate + 100)) {
        int return_rate;

        if (itr->under < 100) { // 猜大
            return_rate = (99 - itr->under); // 最小猜0 itr->under = 0 赔率...., 最大猜 99 itr->under = 99 赔率98倍。
        } else { // 猜小
            return_rate = (itr->under - (100) ); // 最小猜0 itr->under = 100 赔率98倍. 最大猜 99 itr->under = 199, 赔率...
        }  
        action(
                permission_level{_self, N(active)},
                N(eosio.token), N(transfer),
                make_tuple(_self, itr->owner, asset(itr->bet * 98 / return_rate , EOS_SYMBOL),
                    std::string("happy eos dice bonus. The result is: ") + int_to_string(bonus_rate) + std::string(" happyeosslot.com") ))
            .send();

       
    } else {
        if (itr->bet / 200 > 0) {        
            auto tar = eosio::name{itr->owner}.to_string();

            action(
                permission_level{_self, N(active)},
                N(eosio.token), N(transfer),
                make_tuple(_self, N(happyeosslot), asset(itr->bet / 200 , EOS_SYMBOL),
                    std::string("buy for " + tar)))
            .send(); 
        }        
    }
//          static char msg[100];
//       sprintf(msg, "Happy eos slot bonus. dicemaster11.com: %d", bonus/10000); 
    set_roll_result(itr->owner, bonus_rate);
    offers.erase(itr);
}

checksum256 dicemaster11::parse_memo(const std::string &memo) const {
    checksum256 checksum;
    memset(&checksum, 0, sizeof(checksum256));
    for (int i = 0; i < memo.length(); i++) {
        checksum.hash[i & 31] ^= memo[i];
    }
    return checksum;
}

void dicemaster11::set_roll_result(const account_name account, uint64_t roll_number) {
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

void dicemaster11::test(const account_name account, asset eos) {
    require_auth(_self);
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
MY_EOSIO_ABI(dicemaster11, (onTransfer)(init)(reveal)(test))

// generate .abi file
// EOSIO_ABI(dicemaster11, (init)(reveal)(test))