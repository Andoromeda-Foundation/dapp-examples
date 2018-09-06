#include <eosiolib/currency.hpp>
#include <eosiolib/asset.hpp>
#include <math.h>
#include <string>

#define EOS S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using namespace eosio;
using namespace std;

class exchange : public contract {
 public:
  exchange();

  void init();
  void clean();

  void newOrder(account_name account, asset bid, asset ask, account_name issuer); // check order.

  // For test only.
  void test();

 private:
  // @abi table global i64
  struct global {
    uint64_t id = 0;
    uint64_t current_order_id = 0; // Order in one second.
    uint64_t current_log_id = 0; // current log id.

    uint64_t primary_key() const { return id; }
    EOSLIB_SERIALIZE(global, (id)(current_stamp))
  };
  typedef eosio::multi_index<N(global), global> global_index;
  global_index global;

  // @abi table currency i64
  struct currency {
    uint64_t id;
    account_name issuer;
    symbol_type symbol;
    uint64_t primary_key() const { return id; }
    EOSLIB_SERIALIZE(currency, (id)(issuer)(symbol))
  };
  typedef eosio::multi_index<N(currency), currency> currency_index;

  // @abi table price i64
  struct price {  // scope: the target currency id;
    uint64_t price;  // price = Other token amount * precision / EOS token amount
    uint64_t first_id;
    uint64_t last_id;
    int64_t buy; // The total value buy or sell; 1, 0, -1;

    uint64_t primary_key() const { return price; }
    EOSLIB_SERIALIZE(price, (price)(first_id)(last_id))
  };
  typedef eosio::multi_index<N(prices), price> prices;

  // @abi table order i64
  struct order {
    uint64_t id;
    account_name customer;
    uint64_t price;
    int64_t amount;
    uint64_t next_order = 0;

    uint64_t primary_key() const {return id;}
    EOSLIB_SERIALIZE(order, (id)(customer)(price)(amount)(next_order))
  };

  // @abi table txlog i64
  struct txlog {
    uint64_t id;
    account_name buyer; // 买方
    account_name seller; // 卖方
    asset bid; // 供
    asset ask; // 需

    uint64_t primary_key() const {return id;}
    EOSLIB_SERIALIZE(txlog, (id)(buyer)(seller)(bid)(ask))
  };
  typedef eosio::multi_index<N(log), txlog> log_index;
  log_index log;

  static order make_order(account_name account, asset bid, asset ask) const;
  static uint64_t get_price(uint64_t amount_a, uint64_t amount_b) const;
};