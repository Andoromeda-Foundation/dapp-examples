#include <string>

using namespace eosio;
using namespace std;

uint64_t string_to_int(string s) {
    uint64_t z = 0;
    for (int i=0;i<s.size();++i) {
        z += s[i] - '0';
        z *= 10;
    }
    return z;
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

class stringSplitter {
    public:
      stringSplitter(const string& _str) : str(_str) {
          current_position = 0;
      }

      bool eof() {
          return current_position == str.length();
      }

      void skip_empty() {
          while (!eof() && str[current_position] == ' ') current_position ++;
      }

      bool get_char(char* ch) {
          if (!eof()) {
              *ch  = str[current_position++];
              if (*ch == ' ') return false;
              else return true;
          } else return false;
      }

      void get_string(string* result) {
          result->clear();
          skip_empty();
          // if (eof()) return -1;
          //eosio_assert(!eof(), "No enough chars.");
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

// @abi table bets i64
struct rec_bet {
    account_name player;
    account_name referrer;
    asset amount;
//    uint8_t only_odd;
//    uint8_t only_even;    
    uint8_t roll_above;
    uint8_t roll_under;
    checksum256 server_hash;
    checksum256 client_seed;
};

struct rec_reveal {
    account_name player;
    asset amount;
//    uint8_t only_odd;
//    uint8_t only_even;    
    uint8_t roll_above;
    uint8_t roll_under;
    uint8_t random_roll;
    checksum256 server_seed;
    checksum256 client_seed;
    asset payout;  
};

struct rec_buy {
    account_name buyer;
    asset eos;
    asset dmt;
    uint32_t average_price;
};

struct rec_sell {
    account_name seller;
    asset dmt;    
    asset eos;
    uint32_t average_price;
};