#include "pomelo.hpp"

/// @abi action
void pomelo::cancelsell(account_name account, uint64_t id)
{
    require_auth(account);
    auto itr = sellrecords.find(id);
    eosio_assert(itr->account == account, "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");
    // TODO: 返还
    sellrecords.erase(itr);
}

/// @abi action
void pomelo::cancelbuy(account_name account, uint64_t id)
{
    require_auth(account);
    auto itr = buyrecords.find(id);
    eosio_assert(itr->account == account, "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");
    // TODO: 返还
    buyrecords.erase(itr);
}

/// @abi action
void pomelo::buy(account_name account, asset quant, uint64_t total_eos)
{
    require_auth(account);
    eosio_assert(quant.symbol != EOS, "Must buy non-EOS currency");
    eosio_assert(total_eos > 0, "");

    action(
        permission_level{ account, N(active) },
        TOKEN_CONTRACT, N(transfer),
        make_tuple(account, _self, asset(total_eos, EOS), string("transfer"))) // 由合约账号代为管理用于购买代币的EOS
        .send();

    // 生成购买订单
    buyrecord b;
    b.account = account;
    b.asset = quant;
    b.per = (double)total_eos / (double)quant.amount;
    b.total_eos = total_eos;
    do_buy_trade(b);
}

/// @abi action
void pomelo::sell(account_name account, asset quant, uint64_t total_eos)
{
    require_auth(account);
    eosio_assert(quant.symbol != EOS, "Must sale non-EOS currency");
    eosio_assert(total_eos > 0, "");

    action(
        permission_level{ account, N(active) },
        TOKEN_CONTRACT, N(transfer),
        make_tuple(account, _self, quant, string("transfer"))) // 由合约账号代为管理欲出售的代币
        .send();

    // 生成出售订单
    sellrecord s;
    s.account = account;
    s.asset = quant;
    s.per = (double)total_eos / (double)quant.amount;
    s.total_eos = total_eos;
    do_sell_trade(s);
}

// generate .wasm and .wast file
// EOSIO_WAST(pomelo, (cancelbuy)(cancelsell)(buy)(sell))

// generate .abi file
EOSIO_ABI(pomelo, (cancelbuy)(cancelsell)(buy)(sell))