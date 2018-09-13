/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

// dicemaster
/*

class kyubey : public token {
    public:
        kyubey(account_name self) :
        token(self),
        global(_self, _self),
        _market(_self, _self) {}
        
        void buy(const account_name account, asset eos);
        void sell(const account_name account, asset hpy);

        const uint64_t init_quote_balance = 1250 * 10000ll; // 初始保证金 1250 EOS;
        uint64_t get_deposit() const{
            auto market_itr = _market.begin();
            return market_itr->deposit.balance.amount - init_quote_balance;  
        }

        real_type grief_ratio() const;

        // @abi table global i64
        struct global {
            uint64_t id = 0;
            checksum256 hash; // hash of the game seed, 0 when idle.
            uint64_t offerBalance; // All balance in offer list.

            uint64_t primary_key() const { return id; }
            EOSLIB_SERIALIZE(global, (id)(hash)(offerBalance)) 
        };
        
        typedef eosio::multi_index<N(global), global> global_index;
        global_index global;        

        // @abi table market i64
        struct exchange_state {
            uint64_t id = 0;
            asset supply;

            struct connector {
                asset balance;
                double weight = 0.5;
                EOSLIB_SERIALIZE(connector, (balance)(weight))
            };

            connector deposit;

            uint64_t primary_key() const { return id; }

            uint64_t int_sqrt(uint64_t number) {
                uint64_t min = 0, max = 0x7FFFFFFF;
                while (min < max) {
                    uint64_t mid = (min + max) >> 1;
                    if (mid * mid <= number) {
                        if ((mid + 1) * (mid + 1) > number) {
                            return mid;
                        } else {
                            min = mid + 1;
                        }
                    } else {
                        max = mid - 1;
                    }

                }
                return min; // never excuted.
            }     

            asset convert_to_exchange(connector &c, asset in) {
                // 增加输入的EOS
                c.balance.amount += in.amount;
                // 计算增加输入之后的新supply, 按照公式
                // c.balance.amount = (supply_amount / 250000 价格) * (supply_amount / 10000 数量) / 2
                // c.balance.amount = supply_amount * supply_amount / 250000 / 10000 / 2.
                // supply_amount = sqrt(c.balance.amount * 2 * 250000 * 10000);
                int64_t supply_amount = int_sqrt(c.balance.amount * 2 * 250000 * 10000);
                int64_t issued = supply_amount - supply.amount;
                supply.amount = supply_amount;

                c.balance.amount = (supply.amount * supply.amount) / 2 / 250000 / 10000;
                return asset(issued, supply.symbol);
            }

            asset convert_from_exchange(connector &c, asset in) {
                // 每出售250000个HPY价格提升1EOS
                // (((supply.amount / 250000) 上底 + ((supply.amount - in.amount) /250000)下底))
                //  * (in.amount / 10000高) / 2 * 10000(EOS兑换)
                // 现在限制发行250000 HPY 所以这里不会整数溢出
                int64_t eos_return = (((supply.amount << 1) - in.amount) * in.amount / 500000 / 10000);
                supply.amount -= in.amount;
                //supply.amount -= eos_return;
                c.balance.amount = (supply.amount * supply.amount) / 2 / 250000 / 10000;
                return asset(eos_return, c.balance.symbol);
            }

            asset convert(asset from, symbol_type to) {
                if (from.symbol == EOS_SYMBOL && to == HPY_SYMBOL) {
                    return convert_to_exchange(deposit, from);
                } else if (from.symbol == HPY_SYMBOL && to == EOS_SYMBOL) {
                    return convert_from_exchange(deposit, from);
                } else {
                    eosio_assert(false, "Illegal convertion.");
                    return from;
                }
            }

            EOSLIB_SERIALIZE(exchange_state, (supply)(deposit))
        };
        typedef eosio::multi_index<N(market), exchange_state> market;
        market _market;
};
*/
