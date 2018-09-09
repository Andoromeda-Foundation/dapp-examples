#include <eosiolib/currency.hpp>
#include <eosiolib/asset.hpp>
#include <math.h>
#include <string>

#define EOS S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using namespace eosio;
using namespace std;

typedef double real_type;

class eosotcbackup : public contract
{
public:
    eosotcbackup(account_name self) : 
        contract(self) {
    }

    void init(); void clean(); void test();
    void ask(account_name owner, extended_asset bid, extended_asset ask);
    void take(account_name owner, uint64_t order_id, extended_asset bid, extended_asset ask);
    void retrieve(account_name owner, uint64_t order_id);

    void onTransfer(account_name   from,
                    account_name   to,
                    extended_asset quantity,
                    string         memo);  

    void transfer(account_name from,
                  account_name to,
                  asset        quantity,
                  string       memo);                 

    /// @abi table
    struct order {
        uint64_t id;
        account_name owner; // 发起者
        extended_asset bid; // 提供
        extended_asset ask; // 需求
        time timestamp;

        uint64_t primary_key() const {return id;}
        real_type get_price() const {return real_type(ask.amount) / real_type(bid.amount);}        
        EOSLIB_SERIALIZE(order, (id)(owner)(bid)(ask)(timestamp))
    };
    
    typedef eosio::multi_index<N(order), order, 
        indexed_by<N(byprice), const_mem_fun<order, real_type, &order::get_price>>
    > order_index;
};