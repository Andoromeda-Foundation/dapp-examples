#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

class slot_machine : public contract {
    public:
        slot_machine(account_name self) : contract(self),
        global(_self, _self),
        offers(_self, _self) { }

        void init(account_name self, const checksum256& hash) {
            eosio_assert(self == _self, "only contract itself."); 	  

            auto g = global.find(0);
            if (g == global.end()) {
                global.emplace(_self, [&](auto& g){
                        g.id = 0;
                        g.hash = hash;
                        });
            } else {
                global.modify(g, 0, [&](auto& g) {
                        g.hash = hash;
                        });
            }
        }

        void transfer(account_name from, account_name to, asset eos, std::string memo) { // I cannot understand this...
            if (from == _self || to != _self) {
                return;
            }
            require_auth(from);
            eosio_assert(eos.is_valid(), "Invalid token transfer");
            eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed" );    
            eosio_assert(eos.amount > 0, "must bet a positive amount");

            const checksum256 seed = parse_memo(memo);

            offers.emplace(_self, [&](auto& offer) {
                    offer.id = offers.available_primary_key();
                    offer.owner = from;
                    offer.bet = eos.amount;
                    offer.seed = seed;
                    });     
        }  

        void reveal(const account_name host, const checksum256& seed, const checksum256& hash) {
            require_auth(host);
            eosio_assert(host == _self, "Only happyeosslot can reveal the answer.");
            assert_sha256( (char *)&seed, sizeof(seed), (const checksum256 *)& global.begin()->hash );
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

        // @abi table offer i64
        struct offer {
            uint64_t          id;
            account_name      owner;
            uint64_t          bet;
            checksum256       seed;

            uint64_t primary_key() const { return id; }
            EOSLIB_SERIALIZE( offer, (id)(owner)(bet)(seed) )
        };  
        typedef eosio::multi_index< N(offer), offer> offer_index;  
        offer_index offers;

        const int p[8] = {   25,   50, 1200, 1000, 4000, 20000, 50000, 23725};
        const int b[8] = {10000, 5000, 2000, 1000,  500,   200,    10,     1};

        uint64_t get_bonus(uint64_t seed, uint64_t amount) {
            seed %= 100000;
            int i = 0;
            while (seed >= p[i]) {
                seed -= p[i];
                ++i;
            }
            return b[i] * amount / 100;
        }

        uint64_t merge_seed(const checksum256& s1, const checksum256& s2) {
            uint64_t hash = 0, x;
            for (int i = 0; i < 32; ++i) {
                hash ^= (s1.hash[i] ^ s2.hash[i]) << ((i & 7) << 3);
            }
            return hash;
        }

        void deal_with(eosio::multi_index< N(offer), offer>::const_iterator itr, const checksum256& seed) {
            uint64_t bonus = get_bonus(merge_seed(seed, itr->seed), itr.bet);
            action(
                    permission_level{_self, N(active)},
                    N(eosio.token), N(transfer),
                    make_tuple(_self, itr->owner, asset(bonus, EOS_SYMBOL),
                        std::string("Happy eos slot bonus.")))
                .send();
            offers.erase(itr);
        }

        checksum256 parse_memo(const std::string& memo) {
            checksum256 result;
            memset(&result, 0, sizeof(checksum256));
            for (uint i = 0; i < memo.length(); i ++) {
                result.hash[i & 31] ^= memo[i];
            }
            return result;
        }
};

#define EOSIO_ABI_PRO(TYPE, MEMBERS)                                                                                                              \
    extern "C" {                                                                                                                                    \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                                                   \
        {                                                                                                                                               \
            auto self = receiver;                                                                                                                         \
            if (action == N(onerror))                                                                                                                     \
            {                                                                                                                                             \
                eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account");                                        \
            }                                                                                                                                             \
            if ((code == TOKEN_CONTRACT && action == N(transfer)) || (code == self && (action != N(transfer) ))) \
            {                                                                                                                                             \
                TYPE thiscontract(self);                                                                                                                    \
                switch (action)                                                                                                                             \
                {                                                                                                                                           \
                    EOSIO_API(TYPE, MEMBERS)                                                                                                                  \
                }                                                                                                                                           \
            }                                                                                                                                             \
        }                                                                                                                                               \
    }

// generate .wasm and .wast file
EOSIO_ABI_PRO(slot_machine, (transfer)(init)(bet)(reveal))

    // generate .abi file
    // EOSIO_ABI(slot_machine, (transfer)(init)(bet)(reveal))
