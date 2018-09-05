#include <exchange.hpp>

constexpr uint64_t kPRECISION = 1000000;

exchange::exchange() {

}

void exchange::init() {
  require_auth(_self);
}

void exchange::newOrder(account_name account, asset bid, asset ask, account_name issuer) {
  require_auth(account);
  eosio_assert(bid.amount > 0);
  eosio_assert(ask.amount > 0);
  order new_order = make_order(account, bid, ask);
  orders order_table(_self, _self);
  prices price_table(_self, issuer);
  auto price = price_table.find(new_order.price);
  if ((price == price_table.end()) {
      price_table.emplace(0, [&](auto& a){
          a.price = new_order.price; // todo.
      })
  } else {
    if (price->buy * new_order.amount < 0) {
      uint64_t current_id = price->first_id;
      while (current_id) {
        auto order = order_table.get(current_id);
        if (abs(order->amount) <= abs(new_order.amount)) {
          march(new_order.customer, order->customer, order->amount, order->price);
          current_id = order->next_order;
          new_order.amount += order->amount;
          order_table.erase(order);
        } else {
          march(new_order.customer, order->customer, - new_order.amount, order->price);
          order_table.modify(order, 0, [&](auto& o){
              o.amount += new_order.amount;
          });
          new_order.amount = 0;
          break;
        }
      }
      price_table.modify(price, 0, [&](auto& p){
        p.first_id = current_id;
        if (current_id == 0) {
          p.last_id = current_id;
          p.buy = 0;
        }
      });
    }
    if (new_order.amount) {
      // 挂单
      auto g = global.find(0);
      global.modify(g, 0, [&](auto& g){
        new_order->id = ++g.current_order_id;
      });
      order_table.emplace(0, [&](auto& o){
        o = new_order;
      });

      price_table.modify(price, 0, [&](auto& p){
        if (p.last_id == 0) {
          p.first_id = p.last_id = new_order.id;
        } else {
          auto order = order_table.get(p.last_id);
          order_table.modify(order, 0, [&](auto& o){
            o.next_order = new_order.id;
          });
          p.last_id = new_order.id;
        }
        p.buy = new_order.amount;
        p.buy = p.buy / abs(p.buy);
      });
    }
  }
}

void exchange::cancelOrder(account_name account, asset bid, asset ask, account_name issuer) {
  require_auth(account);
  eosio_assert(bid.amount > 0);
  eosio_assert(ask.amount > 0);
  order new_order = make_order(account, bid, ask);

  orders order_table(_self, _self);
  prices price_table(_self, issuer);
  auto price = price_table.find(new_order.price);
  if ((price != price_table.end()) {
      auto last_order = order_table.get(price->first_id);
      if (last_order->customer == account) {
        price_table.modify(price, 0, [&](auto& p){
          p.first_id = last_order->next_order;
          if (p.first_id == 0) {
            p.last_id = 0;
          }
        });
        order_table.erase(last_order);
        return; // clear one order per time to avoid time out.
      }
      uint64_t current_id = last_order->next_order;
      while (current_id) {
        auto order = order_table.get(current_id);
        if (order->customer == account) {
          if (order->next_order == 0) {
            price_table.modify(price, 0, [&](auto& p){
              p.last_id = last_order->id;
            });
          }
          order_table.erase(order);
          return; // clear one order per time to avoid time out.
        }
        last_order = order;
        current_id = last_order->next_order;
      }
  }
}

order exchange::make_order(account_name account, asset bid, asset ask) {
  order result;
  result.customer = account;
  if (bid.symbol == EOS_SYMBOL) {
    result.price = get_price(bid.amount, ask.amount);
    result.amount = bid.symbol * kPRECISION / price;
  } else {
    result.price = get_price(bid.amount, ask.amount);
    result.amount = - bid.amount; // check?
  }
  return result;
}

uint64_t exchange::get_price(uint64_t amount_a, uint64_t amount_b) const {
  uint_t result = amount_a * kPRECISION; // EOS amount;
  eosio_assert(result / kPRECISION == amount_a, "Too big amount");
  return amount_a * kPRECISION / amount_b;
}