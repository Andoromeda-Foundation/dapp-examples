#include <eosiolib/crypto.h>
#include <vector>
#include "happyeos21pt.hpp"

class stringSplitter {
    public:
      stringSplitter(const string& _str) : str(_str) {
          current_position = 0;
      }

      inline bool eof() {
          return current_position == str.length();
      }

      inline void skip_empty() {
          while (!eof() && str[current_position] == ' ') current_position ++;
      }

      inline bool get_char(char* ch) {
          if (!eof()) {
              *ch  = str[current_position++];
              return *ch != ' ' ;
              // if (*ch == ' ') return false;
              // else return true;
          } else return false;
      }

      void get_string(string* result) {
          result->clear();
          skip_empty();
          // if (eof()) return -1;
          eosio_assert(!eof(), "No enough chars.");
          char ch;
          while (get_char(&ch)) {
              *result+= ch;
              //current_position++;
          }
          skip_empty();
      }

      void get_uint(uint64_t* result) {
          skip_empty();
          *result = 0;
          char ch;
          while (get_char(&ch)) {
              eosio_assert(ch >= '0' && ch <= '9', "Should be a valid number");
              *result = *result * 10 + ch - '0';
          }
          skip_empty();
      }
      
    private:
      string str;
      int current_position;
};

void happyeos21pt::send_referal_bonus(const account_name referal, asset eos) {
    if (!is_account(referal)) return;
}

// @abi action
void happyeos21pt::init(const checksum256 &hash) {
    require_auth(_self);
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
}

 // @abi action
void happyeos21pt::test() {
    require_auth(_self);
}


void happyeos21pt::add_bet(const account_name player, const checksum256& seed, const asset eos) {    
    auto itr = games.find(0);
    if (itr == games.end()) {
        games.emplace(_self, [&](auto& g) {
            g.id = games.available_primary_key();
            g.player = player; 
            g.bid = eos.amount;
            g.seed = seed;
        });
    } else {
        games.modify(itr, 0, [&](auto &g) {
            g.bid += eos.amount;
        });
    }
}

void happyeos21pt::join(const account_name player, const account_name referal, asset eos, const checksum256& seed) {
//  require_auth( _self );
    // eosio_assert(bet_number >= 0, "Bet number should bigger or equal to 0."); always true.
    eosio_assert(games.begin() == games.end(), "game already exist.");
    add_bet(player, seed, eos);
    send_referal_bonus(referal, eos);
}

// @abi action
void happyeos21pt::hit(const account_name player) {
    
}

// @abi action
void happyeos21pt::stand(const account_name player) {

}

void happyeos21pt::merge_seed(checksum256 &s1, checksum256 s2) {
    for (int i = 0; i < 32; ++i) {
        s1.hash[i] ^= s2.hash[31-i];
    }
}


bool happyeos21pt::isplayer_win( const uint32_t dPoints, const uint32_t pPoints ) {
    if ( pPoints > 21 ) {
      return false;
    }    
    if ( dPoints > 21 ) {
      return true;
    }
    return pPoints > dPoints;
}
void happyeos21pt::deal_with(game_index::const_iterator itr, const uint32_t dPoints, const uint32_t pPoints) {
    if (isplayer_win(dPoints, pPoints)) {
        // send Winnings
        action(
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, itr->player, itr->bid * 2,
                std::string("win, congraduation!"))
        ).send(); 
    } else {

    }
    games.erase(itr);
}


// 1A 1B 1C 1D
// 2A 2B ..
uint32_t getPointById(uint32_t id) {
    id /= 4;
    if (id < 10) {
        return id + 1;
    }
    return 10; // 10, J, Q, K
}

bool happyeos21pt::verify(const checksum256 &seed, const uint32_t dPoints, const uint32_t pPoints){
    uint32_t dP = 0, cur = 0;
    std::vector<uint32_t> desk;
    for (int i=0;i<52;++i){
        desk.push_back(i);
    }

    while (dP < 16) {
        uint32_t r = cur + seed.hash[cur] % (52 - cur);
        std::swap(desk[cur], desk[r]);
        dP += getPointById(desk.back());
    }

    eosio_assert(dP == dPoints, "there was a problem with dPoints");  
    // todo(minakokojima): verify pPoints;   
    return true;
}

// @abi action
void happyeos21pt::reveal(checksum256 &seed, const uint32_t dPoints, const uint32_t pPoints, const checksum256 &hash) {
    require_auth(_self);
    assert_sha256((char *)&seed, sizeof(seed), (const checksum256 *)&global.begin()->hash);

    auto n = games.available_primary_key();
    if (n == 1){
        for (int i = 0; i < n; ++i) {
            auto itr = games.find(i);
            merge_seed(seed, itr->seed);
            eosio_assert(verify(seed, dPoints, pPoints), "the given seed cannot be verify by the contract");            
            deal_with(itr, dPoints, pPoints);
        }
    } 
    auto itr = global.find(0);
    global.modify(itr, 0, [&](auto &g) {
        g.hash = hash;
    });    
}        


// Rules of play
// On their turn, players must choose whether to
// "hit" (take a card),
// "stand" (end their turn),
// "double" (double wager, take a single card and finish),
// "split" (if the two cards have the same value, separate them to make two hands) or
// "surrender" (give up a half-bet and retire from the game). 

checksum256 happyeos21pt::parse_memo(const std::string &memo) const {
    checksum256 checksum;
    memset(&checksum, 0, sizeof(checksum256));
    for (int i = 0; i < memo.length(); i++) {
        checksum.hash[i & 31] ^= memo[i];
    }
    return checksum;
}

 // @abi action
void happyeos21pt::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) return;
    
    require_auth(from);
    auto player = from;

    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS_SYMBOL, "only core token allowed");
    eosio_assert(eos.amount > 0, "must bet a positive amount");
    
    stringSplitter stream(memo);

    string operation;
    // join safdsa minakokojima
    stream.get_string(&operation);

    if (operation == "join" ) {        
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
        join(player, referal, eos, seed);
    }
}

#define EOSIO_WAST(TYPE, MEMBERS)                                                                                  \
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
// EOSIO_WAST(happyeos21pt, (onTransfer)(init)(test)(hit)(stand)(reveal))

// generate .abi file 
EOSIO_ABI(happyeos21pt, (init)(test)(hit)(stand)(reveal))