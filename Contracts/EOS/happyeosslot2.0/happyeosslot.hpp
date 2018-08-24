/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/eosio.hpp>
#include <string>

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using std::string;
using namespace eosio;

class happyeosslot : public contract {
public:
    happyeosslot(account_name self):contract(self),
                                    global(_self, _self),
                                    offers(_self, _self),
                                    results(_self, _self) {}
    
    void create(account_name issuer, asset maximum_supply);
    
    void issue(account_name to, asset quantity, string memo);
    
    void transfer(account_name from,
                account_name to,
                asset        quantity,
                string       memo);

    void init(account_name self, const checksum256& hash);
    void bet(const account_name account, const uint64_t bet, const checksum256& seed);    
    void sell(uint64_t amount);
    void reveal(const account_name host, const checksum256 &seed, const checksum256 &hash);
    
    inline asset get_supply(symbol_name sym)const;
    inline asset get_balance(account_name owner, symbol_name sym)const;

private:

    // @abi table global i64    
    struct account {
        asset    balance;
        uint64_t primary_key()const { return balance.symbol.name(); }
    };

    // @abi table global i64    
    struct currency_stats {
        asset          supply;
        asset          max_supply;
        account_name   issuer;
        
        uint64_t primary_key()const { return supply.symbol.name(); }
    };

    typedef eosio::multi_index<N(accounts), account> accounts;
    typedef eosio::multi_index<N(stat), currency_stats> stats;
        
    // @abi table global i64
    struct global {
        uint64_t id = 0;
        checksum256 hash; // hash of the game seed, 0 when idle.

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
        account_name owner;
        uint64_t roll_number;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(result, (owner)(roll_number))
    };
    typedef eosio::multi_index<N(result), result> result_index;
    result_index results;      
    
    void sub_balance(account_name owner, asset value);
    void add_balance(account_name owner, asset value, account_name ram_payer);
    uint64_t merge_seed(const checksum256& s1, const checksum256& s2);    
    void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr, const checksum256& seed);
    uint64_t get_bonus(uint64_t seed);
    checksum256 parse_memo(const std::string &memo);

public:
    struct transfer_args {
        account_name  from;
        account_name  to;
        asset         quantity;
        string        memo;
    };
};

asset happyeosslot::get_supply(symbol_name sym)const
{
    stats statstable(_self, sym);
    const auto& st = statstable.get(sym);
    return st.supply;
}

asset happyeosslot::get_balance(account_name owner, symbol_name sym)const
{
    accounts accountstable(_self, owner);
    const auto& ac = accountstable.get(sym);
    return ac.balance;
}

const int p[8] = {   25,   50,  120, 1000, 4000, 20000, 50000, 99999};
const int b[8] = {10000, 5000, 2000, 1000,  500,   200,    10,     1};

uint64_t happyeosslot::get_bonus(uint64_t seed) {
    seed %= 100000;
    int i = 0;
    while (seed >= p[i]) {
        seed -= p[i];
        ++i;
    }
    return b[i];
}

uint64_t happyeosslot::merge_seed(const checksum256 &s1, const checksum256 &s2) {
    uint64_t hash = 0, x;
    for (int i = 0; i < 32; ++i) {
        hash ^= (s1.hash[i] ^ s2.hash[i]) << ((i & 7) << 3);
    }
    return hash;
}

void happyeosslot::deal_with(eosio::multi_index<N(offer), offer>::const_iterator itr, const checksum256 &seed) {
    uint64_t bonus_rate = get_bonus(merge_seed(seed, itr->seed));
    uint64_t bonus = bonus * itr->bet / 100;
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, itr->owner, asset(bonus, EOS_SYMBOL),
                    std::string("Happy eos slot bonus. happyeosslot.com")))
        .send();
    auto p = results.find(itr->owner);
    
    if (p == results.end()) {
        p = results.emplace(_self, [&](auto& result) {
            result.owner = itr->owner;
        });
    }
    results.modify(p, 0, [&](auto& result) {
        result.roll_number = bonus_rate;
    });
    offers.erase(itr);
}

checksum256 happyeosslot::parse_memo(const std::string &memo) { // to bo refind.
    checksum256 checksum;
    memset(&checksum, 0, sizeof(checksum256));
    for (int i = 0; i < memo.length(); i++) {
        checksum.hash[i & 31] ^= memo[i];
    }
    return checksum;
}