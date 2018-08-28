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
        sellrecords(_self, _self),
        txlogs(_self, _self)
    {
    }

    void cancelsell(account_name account, uint64_t id);
    void cancelbuy(account_name account, uint64_t id);
    void buy(account_name account, asset quant, uint64_t total_eos);
    void sell(account_name account, asset quant, uint64_t total_eos);

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

    /// @abi table
    struct txlog {
        uint64_t id;
        uint64_t timestamp;
        account_name seller;
        account_name buyer;
        asset asset;
        uint64_t total_eos;
        double per;

        uint64_t primary_key() const { return id; }
        uint64_t by_timestamp() const { return timestamp; }
        EOSLIB_SERIALIZE(txlog, (id)(timestamp)(seller)(buyer)(asset)(total_eos)(per))

    };
    typedef eosio::multi_index<N(txlog), txlog, indexed_by<N(timestamp), const_mem_fun<txlog, uint64_t, &txlog::by_timestamp>>> txlogs_index;
    txlogs_index txlogs;

    void do_sell_trade(sellrecord s) {
        auto per_index = buyrecords.get_index<N(per)>();
        for (auto itr = per_index.upper_bound(s.per - 0.000000001); itr != per_index.end(); ++itr) {
            // 币种不同则跳过
            if (itr->asset.symbol != s.asset.symbol) {
                continue;
            }
            if (itr->per < s.per) {
                continue;
            }

            if (s.asset.amount >= itr->asset.amount) {
                auto sold_amount = itr->asset.amount;
                auto sold_eos = (uint64_t)(s.per * itr->asset.amount);
                insert_txlog(itr->account, s.account, asset(sold_amount, itr->asset.symbol), sold_eos, s.per);
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

                insert_txlog(itr->account, s.account, asset(sold_amount, itr->asset.symbol), sold_eos, s.per);

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

                    insert_txlog(b.account, itr->account, asset(sold_amount, itr->asset.symbol), sold_eos, itr->per);

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

                    insert_txlog(b.account, itr->account, asset(sold_amount, itr->asset.symbol), sold_eos, itr->per);

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

    void maintain_txlogs() {
        auto timestamp_index = txlogs.get_index<N(timestamp)>();
        for (auto itr = timestamp_index.lower_bound(current_time() - 1000 * 1000 * 60 * 60); itr != timestamp_index.end(); ++itr) {
            timestamp_index.erase(itr);
        }
    }

    void insert_txlog(account_name buyer, account_name seller, asset quant, uint64_t total_eos, double per) {
        maintain_txlogs();
        txlogs.emplace(_self, [&](auto& l) {
            l.id = txlogs.available_primary_key();
            l.timestamp = current_time();
            l.buyer = buyer;
            l.seller = seller;
            l.asset = quant;
            l.total_eos = total_eos; // 剩余的EOS
            l.per = per;
        });
    }
};

EOSIO_ABI(pomelo, (cancelbuy)(cancelsell)(buy)(sell))