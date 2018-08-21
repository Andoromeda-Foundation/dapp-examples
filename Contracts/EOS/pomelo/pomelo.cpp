
#include <eosiolib/currency.hpp>
#include <math.h>
#include <string>

#define EOS S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using namespace eosio;
using namespace std;


typedef double real_type;

class pomelo : public contract
{
public:
  pomelo(account_name self)
      : contract(self),
      trades(_self, _self)
  {
  }

  /// @abi action
  void cancel(account_name account)
  {
    require_auth(account);
    auto status_index = trades.get_index<N(status)>();
    for (auto itr = status_index.lower_bound(1); itr != status_index.end(); ++itr) {
      if (itr -> status != 1) { // 如果迭代器进入了比1大的状态则结束迭代
        break;
      }

      if (itr -> account == account) {
        auto quant = itr -> asset;
        action( // 退还代币
          permission_level{_self, N(active)},
          TOKEN_CONTRACT, N(transfer),
          make_tuple(_self, account, quant, string("back")))
          .send();
        status_index.erase(itr);
      }
    }
    for (auto itr = status_index.lower_bound(0); itr != status_index.end(); ++itr) {
      if (itr -> status != 0) { // 如果迭代器进入了比0大的状态则结束迭代
        break;
      }

      if (itr -> account == account) {
        asset quant;
        quant.symbol = EOS;
        quant.amount = itr -> total_eos;
        action( // 退还EOS
          permission_level{_self, N(active)},
          TOKEN_CONTRACT, N(transfer),
          make_tuple(_self, account, quant, string("back")))
          .send();
        status_index.erase(itr);
      }
    }
  }

  /// @abi action
  void buy(account_name account, asset quant, uint64_t total_eos)
  {
    require_auth(account);
    eosio_assert(quant.symbol != EOS, "Must buy non-EOS currency");
    eosio_assert(total_eos > 0, "");

    action(
      permission_level{account, N(active)},
      TOKEN_CONTRACT, N(transfer),
      make_tuple(account, _self, quant, string("transfer"))) // 由合约账号代为管理用于购买代币的EOS
      .send();
      trade t;
      t.account = account;
      t.asset = quant;
      t.status = 0;
      t.total_eos = total_eos;
      do_trade(t);
  }

  /// @abi action
  void sell(account_name account, asset quant, uint64_t total_eos)
  {
    require_auth(account);
    eosio_assert(quant.symbol != EOS, "Must sale non-EOS currency");
    eosio_assert(total_eos > 0, "");

    action(
      permission_level{account, N(active)},
      TOKEN_CONTRACT, N(transfer),
      make_tuple(account, _self, quant, string("transfer"))) // 由合约账号代为管理欲出售的代币
      .send();
      trade t;
      t.account = account;
      t.asset = quant;
      t.status = 1;
      t.total_eos = total_eos;
      do_trade(t);
  }

private:
    /// @abi table
    struct trade {
      uint64_t id = 0;
      uint64_t account;
      asset asset;
      uint64_t total_eos;
      uint64_t status;

      uint64_t primary_key() const { return id; }
      uint64_t by_status() const { return status; }
      EOSLIB_SERIALIZE(trade, (id)(account)(asset)(total_eos)(status))
  
    };
    typedef eosio::multi_index<N(trade), trade, indexed_by<N(status), const_mem_fun<trade, uint64_t, &trade::by_status>>> trade_index;
    trade_index trades;

    void do_trade(trade trade) {
      auto status_index = trades.get_index<N(status)>();
      if (trade.status == 0) {
        for (auto itr = status_index.lower_bound(1); itr != status_index.end() && trade.asset.amount > 0; ++itr) {
          if (itr -> status != 1) { // 如果迭代器进入了比1大的状态则结束迭代
            break;
          }

          auto sell_unit = (double)itr -> total_eos / (double)itr -> asset.amount;
          auto buy_unit = (double)trade.total_eos / (double)trade.asset.amount;
          if (sell_unit > buy_unit) {
            continue;
          }

          auto trade_amount = itr -> asset.amount - trade.asset.amount; // 计算出售者资源是否足够
          if (trade_amount >= 0) { // 出售者的资源比购买者欲购买数量多
            status_index.modify(itr, 0, [&] (auto& t) {
              t.asset.amount = trade_amount;
              t.total_eos -= trade_amount * sell_unit; // 将出售的资源标记售出，剩余部分继续售卖
            });
            trade.asset.amount = 0; // 购买的订单减少相应的数额
            trade.total_eos -= trade_amount * sell_unit; // 计算余额，稍后退还给购买者
            asset a;
            a.symbol = EOS;
            a.amount = sell_unit * trade_amount;
            action( // 给出售者转账EOS
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, itr -> account, a, string("sell")))
              .send();
            a.symbol = trade.asset.symbol;
            a.amount = trade_amount;
            action( // 给购买者转账代币
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, trade.account, a, string("buy")))
              .send();
          } 
          else { // 出售者的资源不足完成本笔购买订单
            asset a, b;
            a.symbol = EOS;
            a.amount = itr -> total_eos;
            b = itr -> asset;
            status_index.modify(itr, 0, [&] (auto& t) {
              t.asset.amount = 0;
              t.total_eos = 0; // 销售单完成
              t.status = 2;
            });
            action( // 给出售者转账EOS
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, itr -> account, a, string("sell")))
              .send();
            action( // 给购买者转账代币
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, trade.account, b, string("buy")))
              .send();
            trade.asset.amount -= b.amount; // 本单剩余购买数量减少
            trade.total_eos -= a.amount;
          }
        }
        if (trade.asset.amount == 0) { // 判断本购买单是否完成
          if (trade.total_eos > 0) { // 购买用的EOS还有剩余将退还
            asset a;
            a.symbol = EOS;
            a.amount = trade.total_eos;
            action( // 给购买者转账代币
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, trade.account, a, string("back")))
              .send();
          }
        }
        else { // 在table中记录为未完成的购买订单，将在有新售卖单发生时尝试继续完成。
          trades.emplace(trade.account, [&] (auto& t) {
            t.asset.symbol = trade.asset.symbol;
            t.asset.amount = trade.asset.amount;
            t.total_eos = trade.total_eos; // 销售单完成
            t.status = 0;
          });
        }
      }
      else if (trade.status == 1) {
        for (auto itr = status_index.lower_bound(0); itr != status_index.end() && trade.asset.amount > 0; ++itr) {
          if (itr -> status != 0) { // 如果迭代器进入了比0大的状态则结束迭代
            break;
          }

          auto buy_unit = (double)itr -> total_eos / (double)itr -> asset.amount;
          auto sell_unit = (double)trade.total_eos / (double)trade.asset.amount;
          if (sell_unit > buy_unit) {
            continue;
          }

          auto trade_amount = trade.asset.amount - itr -> asset.amount; // 计算出售者资源是否足够
          if (trade_amount >= 0) { // 出售者的资源比购买者欲购买数量多
            trade.asset.amount -= trade_amount;
            trade.total_eos -= trade_amount * sell_unit;

            status_index.modify(itr, 0, [&] (auto& t) {
              t.asset.amount = 0; // 购买的订单减少相应的数额
              t.total_eos -= trade_amount * sell_unit; // 计算余额，稍后退还给购买者
            });
            asset a;
            a.symbol = EOS;
            a.amount = sell_unit * trade_amount;
            action( // 给出售者转账EOS
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, trade.account, a, string("sell")))
              .send();
            a.symbol = trade.asset.symbol;
            a.amount = trade_amount;
            action( // 给购买者转账代币
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, itr -> account, a, string("buy")))
              .send();
          } 
          else { // 出售者的资源不足完成本笔购买订单
            asset a, b;
            a.symbol = EOS;
            a.amount = trade.total_eos;
            b = trade.asset;
            trade.status = 2;
            trade.total_eos = 0;
            trade.asset.amount = 0;
            
            action( // 给出售者转账EOS
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, trade. account, a, string("sell")))
              .send();
            action( // 给购买者转账代币
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, itr -> account, b, string("buy")))
              .send();
              
            status_index.modify(itr, 0, [&] (auto& t) {
              t.asset.amount -= b.amount;
              t.total_eos -= a.amount;
            });
          }
        }
        if (trade.asset.amount > 0) { // 判断本购买单是否完成
          trades.emplace(trade.account, [&] (auto& t) {
            t.asset.symbol = trade.asset.symbol;
            t.asset.amount = trade.asset.amount;
            t.total_eos = trade.total_eos;
            t.status = 1;
          });
        }
      }
      else {
        eosio_assert(false, "Invalid trade status");
      }
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
    if ((code == TOKEN_CONTRACT && action == N(transfer)) || (code == self && (action == N(sell) || action == N(destroy) || action == N(claim)))) \
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
// EOSIO_ABI_PRO(itegame, (transfer)(sell)(destroy)(claim))

// generate .abi file
EOSIO_ABI(pomelo, (cancel)(buy)(sell))