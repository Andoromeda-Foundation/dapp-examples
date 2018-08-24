/**
 *  @dev minakokojima
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/eosio.hpp>
#include <cmath>
#include <string>

#define EOS_SYMBOL S(4, EOS)
#define HPY_SYMBOL S(4, HPY)
#define TOKEN_CONTRACT N(eosio.token)

typedef double real_type;

using std::string;
using namespace eosio;

class token : public contract {
    public:
        token( account_name self ):contract(self){}

        void _create( account_name issuer,
                    asset        maximum_supply);

        void _issue( account_name to, asset quantity, string memo );
        void _burn( account_name from, asset quantity );

        void _transfer( account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo );
    
    
        inline asset get_supply( symbol_name sym )const;
        
        inline asset get_balance( account_name owner, symbol_name sym )const;

    private:
        // @abi table accounts i64    
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

class tradeableToken : public token {
    public:    
    tradeableToken(account_name self):token(self), _market(_self, _self) {}
    void buy(const account_name account, asset eos);
    void sell(const account_name account, asset hpy);

    /**
        *  Uses Bancor math to create a 50/50 relay between two asset types. The state of the
        *  bancor exchange is entirely contained within this struct. There are no external
        *  side effects associated with using this API.
        *  Love BM. Love Bancor.
        */
    private:
    // @abi table market i64    
    struct exchange_state
    {
        uint64_t id = 0;

        asset supply;

        struct connector
        {
        asset balance;
        double weight = .5;

        EOSLIB_SERIALIZE(connector, (balance)(weight))
        };

        connector base;
        connector quote;

        uint64_t primary_key() const { return id; }

        asset convert_to_exchange(connector &c, asset in)
        {
        real_type R(supply.amount);
        real_type C(c.balance.amount + in.amount);
        real_type F(c.weight / 1000.0);
        real_type T(in.amount);
        real_type ONE(1.0);

        real_type E = -R * (ONE - pow(ONE + T / C, F));
        int64_t issued = int64_t(E);

        supply.amount += issued;
        c.balance.amount += in.amount;

        return asset(issued, supply.symbol);
        }

        asset convert_from_exchange(connector &c, asset in)
        {
        eosio_assert(in.symbol == supply.symbol, "unexpected asset symbol input");

        real_type R(supply.amount - in.amount);
        real_type C(c.balance.amount);
        real_type F(1000.0 / c.weight);
        real_type E(in.amount);
        real_type ONE(1.0);

        real_type T = C * (pow(ONE + E / R, F) - ONE);
        int64_t out = int64_t(T);

        supply.amount -= in.amount;
        c.balance.amount -= out;

        return asset(out, c.balance.symbol);
        }

        asset convert(asset from, symbol_type to)
        {
        auto sell_symbol = from.symbol;
        auto ex_symbol = supply.symbol;
        auto base_symbol = base.balance.symbol;
        auto quote_symbol = quote.balance.symbol;

        if (sell_symbol != ex_symbol)
        {
            if (sell_symbol == base_symbol)
            {
            from = convert_to_exchange(base, from);
            }
            else if (sell_symbol == quote_symbol)
            {
            from = convert_to_exchange(quote, from);
            }
            else
            {
            eosio_assert(false, "invalid sell");
            }
        }
        else
        {
            if (to == base_symbol)
            {
            from = convert_from_exchange(base, from);
            }
            else if (to == quote_symbol)
            {
            from = convert_from_exchange(quote, from);
            }
            else
            {
            eosio_assert(false, "invalid conversion");
            }
        }

        if (to != from.symbol)
            return convert(from, to);

        return from;
        }

        EOSLIB_SERIALIZE(exchange_state, (supply)(base)(quote))
    };

    typedef eosio::multi_index<N(market), exchange_state> market;
    market _market;    
};

class happyeosslot : public tradeableToken {
public:
    happyeosslot(account_name self):tradeableToken(self),
                                    global(_self, _self),
                                    offers(_self, _self),
                                    results(_self, _self) {}

    void create( account_name issuer,
                asset        maximum_supply);

    void issue( account_name to, asset quantity, string memo );
    void transfer(account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo);  
                                     
    void ontransfer(account_name from,
                account_name to,
                asset        quantity,
                string       memo);

    void init(account_name self, const checksum256& hash);

    void reveal(const account_name host, const checksum256 &seed, const checksum256 &hash);
 
private:
        
    // @abi table global i64
    struct global {
        uint64_t id = 0;
        checksum256 hash; // hash of the game seed, 0 when idle.

        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(global, (id)(hash))
    };
    typedef eosio::multi_index<N(global), global> global_index;
    global_index global;  

    // @abi table offers i64
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

    // @abi table results i64
    struct result {
        account_name owner;
        uint64_t roll_number;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(result, (owner)(roll_number))
    };
    typedef eosio::multi_index<N(result), result> result_index;
    result_index results;
    
    void bet(const account_name account, asset bet, const checksum256& seed);
    uint64_t merge_seed(const checksum256& s1, const checksum256& s2);    
    void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr, const checksum256& seed);
    uint64_t get_bonus(uint64_t seed);
    checksum256 parse_memo(const std::string &memo);
};

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
    
    results.modify(p, 0, [&](auto& result) {
        result.roll_number = bonus_rate;
    });
    offers.erase(itr);
}

checksum256 happyeosslot::parse_memo(const std::string &memo) { // to bo refine.
    checksum256 checksum;
    memset(&checksum, 0, sizeof(checksum256));
    for (int i = 0; i < memo.length(); i++) {
        checksum.hash[i & 31] ^= memo[i];
    }
    return checksum;
}