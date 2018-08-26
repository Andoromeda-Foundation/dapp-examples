
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
  void cancel(account_name account, uint64_t id)
  {
    require_auth(account);
    auto itr = trades.find(id);
    eosio_assert(itr -> account == account, "Account does not match");
    eosio_assert(itr -> id == id, "Trade id is not found");
    trades.erase(itr);
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
      make_tuple(account, _self, asset(total_eos, EOS), string("transfer"))) // 由合约账号代为管理用于购买代币的EOS
      .send();
    
    // 生成购买订单
    buy b;
    b.account = account;
    b.asset = quant;
    b.per = (double)total_eos / (double)quant.amount;
    b.total_eos = total_eos;
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

    // 生成出售订单
    sell s;
    s.account = account;
    s.asset = quant;
    s.per = (double)total_eos / (double)quant.amount;
    s.total_eos = total_eos;
  }

private:
    /// @abi table
    struct buy {
      uint64_t id;
      account_name account;
      asset asset;
      uint64_t total_eos;
      double per;

      uint64_t primary_key() const { return id; }
      uint64_t by_per() const { return per; }
      EOSLIB_SERIALIZE(buy, (id)(account)(asset)(total_eos)(per))
  
    };
    typedef eosio::multi_index<N(trade), trade, indexed_by<N(per), const_mem_fun<trade, uint64_t, &trade::by_per>>> buy_index;
    buy_index buys;

    
    /// @abi table
    struct sell {
      uint64_t id;
      account_name account;
      asset asset;
      uint64_t total_eos;
      double per;

      uint64_t primary_key() const { return id; }
      uint64_t by_per() const { return per; }
      EOSLIB_SERIALIZE(sell, (id)(account)(asset)(total_eos)(per))
  
    };
    typedef eosio::multi_index<N(trade), trade, indexed_by<N(per), const_mem_fun<trade, uint64_t, &trade::by_per>>> sells_index;
    sells_index sells;

    void do_sell_trade(sell s) {
      auto per_index = buys.get_index<N(per)>();
      for (auto itr = per_index.lower_bound(s.per); itr != per_index.end(); ++itr) {
        // 币种不同则跳过
        if (itr -> asset.symbol != s.asset.symbol) {
          continue;
        }

        if (s.asset.amount >= itr -> asset.amount) {
          auto sold_amount = itr -> asset.amount;
          auto sold_eos = (uint64_t)(s.per * itr -> asset.amount);
          s.asset.amount -= sold_amount;
          s.total_eos -= sold_eos;
          auto eos_left = itr -> total_eos - sold_eos;
          per_index.erase(itr);
          
          action(
            permission_level{_self, N(active)},
            TOKEN_CONTRACT, N(transfer),
            make_tuple(_self, itr -> account, asset(sold_amount, s.asset.symbol), string("transfer"))) // 由合约账号代为管理欲出售的代币
            .send();
          action(
            permission_level{_self, N(active)},
            TOKEN_CONTRACT, N(transfer),
            make_tuple(_self, s.account, asset(sold_eos, EOS), string("transfer"))) // 由合约账号代为管理欲出售的代币
            .send();
          if (eos_left) {
            action(
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, itr -> account, asset(eos_left, EOS), string("transfer"))) // 将剩余购买款返还给买家
              .send();
          }
        }
        else {
          auto sold_amount = s.asset.amount;
          auto sold_eos = s.total_eos;
          per_index.modify(itr, 0, [&] (auto& t) {
            t.asset.amount -= sold_amount;
            t.total_eos -= sold_eos;
            t.per = (double)t.asset.amount / (double)t.total_eos;
          });
          
          action(
            permission_level{_self, N(active)},
            TOKEN_CONTRACT, N(transfer),
            make_tuple(_self, itr -> account, asset(sold_amount, s.asset.symbol), string("transfer"))) // 由合约账号代为管理欲出售的代币
            .send();
          action(
            permission_level{_self, N(active)},
            TOKEN_CONTRACT, N(transfer),
            make_tuple(_self, s.account, asset(sold_eos, EOS), string("transfer"))) // 由合约账号代为管理欲出售的代币
            .send();

          return;
        }
      }
      if (s.asset.amount > 0) {
        sells.emplace(0, [&] (auto& t) {
          t.id = sells.available_primary_key();
          t.account = s.account;
          t.asset.symbol = s.asset.symbol;
          t.asset.amount = s.asset.amount;
          t.total_eos = s.total_eos; // 剩余的EOS
          t.per = s.per;
        });
      }
    }

    void do_buy_trade(buy b) {
      auto per_index = sells.get_index<N(per)>();
      auto itr = per_index.upper_bound(b.per);
      bool is_end;
      do {
        is_end = true;
        for (auto itr = per_index.upper_bound(b.per); itr != per_index.end(); ++itr) {
          // 币种不同则跳过
          if (itr -> asset.symbol != b.asset.symbol) {
            continue;
          }

          if (b.asset.amount <= itr -> asset.amount) {
            auto sold_amount = b.asset.amount;
            auto sold_eos = (uint64_t)(itr -> per * b.asset.amount);
            per_index.modify(itr, 0, [&] (auto& t) {
              t.asset.amount -= sold_amount;
              t.total_eos -= sold_eos;
            });
            
            auto eos_left = b.total_eos - sold_eos;
            
            action(
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, b.account, asset(sold_amount, s.asset.symbol), string("transfer")))
              .send();
            action(
              permission_level{_self, N(active)},
              TOKEN_CONTRACT, N(transfer),
              make_tuple(_self, itr -> account, asset(sold_eos, EOS), string("transfer")))
              .send();

            if (eos_left) {
              action(
                permission_level{_self, N(active)},
                TOKEN_CONTRACT, N(transfer),
                make_tuple(_self, b.account, asset(eos_left, EOS), string("transfer"))) // 将剩余购买款返还给买家
                .send();
            }

            return;
        }
        else {
          auto sold_amount = itr -> asset.amount;
          auto sold_eos = (uint64_t)(itr -> per * itr -> asset.amount);
          b.asset.amount -= sold_amount;
          b.total_eos -= sold_eos;
          action(
            permission_level{_self, N(active)},
            TOKEN_CONTRACT, N(transfer),
            make_tuple(_self, b.account, asset(sold_amount, s.asset.symbol), string("transfer")))
            .send();
          action(
            permission_level{_self, N(active)},
            TOKEN_CONTRACT, N(transfer),
            make_tuple(_self, itr -> account, asset(sold_eos, EOS), string("transfer")))
            .send();
          b.per = (double)b.asset.amount / (double)b.total_eos;
          is_end = false;
        }
      }
    }
    while(!is_end);
    if (b.asset.amount > 0) {
      buys.emplace(0, [&] (auto& t) {
        t.id = buys.available_primary_key();
        t.account = s.account;
        t.asset.symbol = s.asset.symbol;
        t.asset.amount = s.asset.amount;
        t.total_eos = s.total_eos; // 剩余的EOS
        t.per = s.per;
      });
    }
};

EOSIO_ABI(pomelo, (cancel)(buy)(sell))