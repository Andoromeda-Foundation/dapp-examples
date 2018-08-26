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
        buyrecords(_self, _self),
        sellrecords(_self, _self)
    {
    }

    /// @abi action
    void cancelsell(account_name account, uint64_t id)
    {
        require_auth(account);
        auto itr = sellrecords.find(id);
        eosio_assert(itr->account == account, "Account does not match");
        eosio_assert(itr->id == id, "Trade id is not found");
        // TODO: 返还
        sellrecords.erase(itr);
    }

    /// @abi action
    void cancelbuy(account_name account, uint64_t id)
    {
        require_auth(account);
        auto itr = buyrecords.find(id);
        eosio_assert(itr->account == account, "Account does not match");
        eosio_assert(itr->id == id, "Trade id is not found");
        // TODO: 返还
        buyrecords.erase(itr);
    }

    /// @abi action
    void buy(account_name account, asset quant, uint64_t total_eos)
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
    void sell(account_name account, asset quant, uint64_t total_eos)
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

private:
    /// @abi table
    struct buyrecord {
        uint64_t id;
        account_name account;
        asset asset;
        uint64_t total_eos;
        double per;

        uint64_t primary_key() const { return id; }
        uint64_t by_per() const { return per; }
        EOSLIB_SERIALIZE(buyrecord, (id)(account)(asset)(total_eos)(per))

    };
    typedef eosio::multi_index<N(buyrecord), buyrecord, indexed_by<N(per), const_mem_fun<buyrecord, uint64_t, &buyrecord::by_per>>> buyrecords_index;
    buyrecords_index buyrecords;

    /// @abi table
    struct sellrecord {
        uint64_t id;
        account_name account;
        asset asset;
        uint64_t total_eos;
        double per;

        uint64_t primary_key() const { return id; }
        uint64_t by_per() const { return per; }
        EOSLIB_SERIALIZE(sellrecord, (id)(account)(asset)(total_eos)(per))

    };
    typedef eosio::multi_index<N(sellrecord), sellrecord, indexed_by<N(per), const_mem_fun<sellrecord, uint64_t, &sellrecord::by_per>>> sellrecords_index;
    sellrecords_index sellrecords;

    void do_sell_trade(sellrecord s) {
        auto per_index = buyrecords.get_index<N(per)>();
        for (auto itr = per_index.upper_bound(s.per); itr != per_index.end(); ++itr) {
            // 币种不同则跳过
            if (itr->asset.symbol != s.asset.symbol) {
                continue;
            }

            if (s.asset.amount >= itr->asset.amount) {
                auto sold_amount = itr->asset.amount;
                auto sold_eos = (uint64_t)(s.per * itr->asset.amount);
                s.asset.amount -= sold_amount;
                s.total_eos -= sold_eos;
                auto eos_left = itr->total_eos - sold_eos;
                per_index.erase(itr);

                action(
                    permission_level{ _self, N(active) },
                    TOKEN_CONTRACT, N(transfer),
                    make_tuple(_self, itr->account, asset(sold_amount, s.asset.symbol), string("transfer"))) // 由合约账号代为管理欲出售的代币
                    .send();
                action(
                    permission_level{ _self, N(active) },
                    TOKEN_CONTRACT, N(transfer),
                    make_tuple(_self, s.account, asset(sold_eos, EOS), string("transfer"))) // 由合约账号代为管理欲出售的代币
                    .send();
                if (eos_left > 0) {
                    action(
                        permission_level{ _self, N(active) },
                        TOKEN_CONTRACT, N(transfer),
                        make_tuple(_self, itr->account, asset(eos_left, EOS), string("transfer"))) // 将剩余购买款返还给买家
                        .send();
                }
            }
            else {
                auto sold_amount = s.asset.amount;
                auto sold_eos = s.total_eos;
                per_index.modify(itr, 0, [&](auto& t) {
                    t.asset.amount -= sold_amount;
                    t.total_eos -= sold_eos;
                    t.per = (double)t.asset.amount / (double)t.total_eos;
                });

                action(
                    permission_level{ _self, N(active) },
                    TOKEN_CONTRACT, N(transfer),
                    make_tuple(_self, itr->account, asset(sold_amount, s.asset.symbol), string("transfer"))) // 由合约账号代为管理欲出售的代币
                    .send();
                action(
                    permission_level{ _self, N(active) },
                    TOKEN_CONTRACT, N(transfer),
                    make_tuple(_self, s.account, asset(sold_eos, EOS), string("transfer"))) // 由合约账号代为管理欲出售的代币
                    .send();

                return;
            }
            if (s.asset.amount == 0) {
                break;
            }
        }
        if (s.asset.amount > 0) {
            sellrecords.emplace(_self, [&](auto& t) {
                t.id = sellrecords.available_primary_key();
                t.account = s.account;
                t.asset.symbol = s.asset.symbol;
                t.asset.amount = s.asset.amount;
                t.total_eos = s.total_eos; // 剩余的EOS
                t.per = s.per;
            });
        }
    }

    void do_buy_trade(buyrecord b) {
        auto per_index = sellrecords.get_index<N(per)>();
        auto itr = per_index.lower_bound(b.per);
        bool is_end;
        do {
            is_end = true;
            for (auto itr = per_index.lower_bound(b.per); itr != per_index.end(); ++itr) {
                // 币种不同则跳过
                if (itr->asset.symbol != b.asset.symbol) {
                    continue;
                }

                if (b.per < itr->per)
                {
                    continue;
                }

                if (b.asset.amount < itr->asset.amount) {
                    auto sold_amount = b.asset.amount;
                    auto sold_eos = (uint64_t)(itr->per * b.asset.amount);
                    per_index.modify(itr, 0, [&](auto& t) {
                        t.asset.amount -= sold_amount;
                        t.total_eos -= sold_eos;
                    });

                    auto eos_left = b.total_eos - sold_eos;

                    action(
                        permission_level{ _self, N(active) },
                        TOKEN_CONTRACT, N(transfer),
                        make_tuple(_self, b.account, asset(sold_amount, b.asset.symbol), string("transfer")))
                        .send();
                    action(
                        permission_level{ _self, N(active) },
                        TOKEN_CONTRACT, N(transfer),
                        make_tuple(_self, itr->account, asset(sold_eos, EOS), string("transfer")))
                        .send();

                    if (eos_left > 0) {
                        action(
                            permission_level{ _self, N(active) },
                            TOKEN_CONTRACT, N(transfer),
                            make_tuple(_self, b.account, asset(eos_left, EOS), string("transfer"))) // 将剩余购买款返还给买家
                            .send();
                    }

                    return;
                }
                else {
                    auto sold_amount = itr->asset.amount;
                    auto sold_eos = (uint64_t)(itr->per * itr->asset.amount);
                    b.asset.amount -= sold_amount;
                    b.total_eos -= sold_eos;
                    per_index.erase(itr);

                    action(
                        permission_level{ _self, N(active) },
                        TOKEN_CONTRACT, N(transfer),
                        make_tuple(_self, b.account, asset(sold_amount, b.asset.symbol), string("transfer")))
                        .send();
                    action(
                        permission_level{ _self, N(active) },
                        TOKEN_CONTRACT, N(transfer),
                        make_tuple(_self, itr->account, asset(sold_eos, EOS), string("transfer")))
                        .send();
                    b.per = (double)b.asset.amount / (double)b.total_eos;
                    is_end = false;
                }
                if (b.asset.amount == 0) {
                    is_end = true;
                    break;
                }
            }
        } while (!is_end);

        if (b.asset.amount > 0) {
            buyrecords.emplace(_self, [&](auto& t) {
                t.id = buyrecords.available_primary_key();
                t.account = b.account;
                t.asset.symbol = b.asset.symbol;
                t.asset.amount = b.asset.amount;
                t.total_eos = b.total_eos; // 剩余的EOS
                t.per = b.per;
            });
        }
    }
};

EOSIO_ABI(pomelo, (cancelbuy)(cancelsell)(buy)(sell))