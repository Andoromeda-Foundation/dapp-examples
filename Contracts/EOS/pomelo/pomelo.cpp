#include "pomelo.hpp"

/*
 #include <eosio/chain/asset.hpp>
// #include <fc/reflect/variant.hpp>


asset from_string(const string& from)
{
   // try {
    string s = from ; //fc::trim(from);
    // Find space in order to split amount and symbol
    auto space_pos = s.find(' ');
    EOS_ASSERT((space_pos != string::npos), asset_type_exception, "Asset's amount and symbol should be separated with space");
    auto symbol_str = s.substr(space_pos + 1);
    auto amount_str = s.substr(0, space_pos);

    // Ensure that if decimal point is used (.), decimal fraction is specified
    auto dot_pos = amount_str.find('.');
    if (dot_pos != string::npos) {
        EOS_ASSERT((dot_pos != amount_str.size() - 1), asset_type_exception, "Missing decimal fraction after decimal point");
    }

    inline std::string to_string( uint32_t field ) {
        return std::string("i32 : ")+std::to_string(field);
    }
    inline std::string to_string( uint64_t field ) {
        return std::string("i64 : ")+std::to_string(field);
    }

      // Parse symbol
    string precision_digit_str;
    if (dot_pos != string::npos) {
        precision_digit_str = eosio::chain::to_string(amount_str.size() - dot_pos - 1);
    } else {
        precision_digit_str = "0";
    }

      string symbol_part = precision_digit_str + ',' + symbol_str;
      symbol sym = symbol::from_string(symbol_part);

      // Parse amount
      int64_t int_part, fract_part;
      if (dot_pos != string::npos) {
         //int_part = fc::to_int64(amount_str.substr(0, dot_pos));
         //fract_part = fc::to_int64(amount_str.substr(dot_pos + 1));
         if (amount_str[0] == '-') fract_part *= -1;
      } else {
         //int_part = fc::to_int64(amount_str);
      }

      int64_t amount = int_part;
      amount *= int64_t(sym.precision());
      amount += fract_part;

      return asset(amount.value, sym);
   }
   //FC_CAPTURE_LOG_AND_RETHROW( (from) )
}



asset from_string(const string& from)
{

}
*/

/// @abi action
void pomelo::init()
{

}

/// @abi action
void pomelo::test()
{

}

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
void pomelo::buy(account_name account, string quant, uint64_t total_eos)
{
    require_auth(account);
//    eosio_assert(quant.is_valid(), "Invalid token transfer");
  //  eosio_assert(quant.symbol != EOS, "Must buy non-EOS currency");

    eosio_assert(total_eos > 0, "");

/*
    action(
        permission_level{ account, N(active) },
        TOKEN_CONTRACT, N(transfer),
        make_tuple(account, _self, asset(total_eos, EOS), string("transfer"))) // 由合约账号代为管理用于购买代币的EOS
        .send();
*/        

    // 生成购买订单
    buyrecord b;
    b.account = account;
    b.target = quant;
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

 // @abi action
void pomelo::onTransfer(account_name from, account_name to, asset eos, std::string memo) {        
    if (to != _self) return;    
    require_auth(from);
    eosio_assert(eos.is_valid(), "Invalid token transfer");
    eosio_assert(eos.symbol == EOS, "only core token allowed"); // To be refine.
    eosio_assert(eos.amount > 0, "must bet a positive amount");
    if (memo.find("buy") != string::npos) {
        memo.erase(0, 4);
        buy(from, memo, eos.amount);
        // buy(from, eos, eos.amount);
    } else {	
        // sell 
    }
}

// "1.0000 EOS"

// boost
// fc

// throw
// webassmebly


#define EOSIO_WAST(TYPE, MEMBERS)                                                                                  \
    extern "C"                                                                                                       \
    {                                                                                                                \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                \
        {                                                                                                            \
                                                                                                                     \
            auto self = receiver;                                                                                    \
            if (action == N(onerror))                                                                                \
            {                                                                                                        \
                eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
            }                                                                                                        \
            if (code == TOKEN_CONTRACT && action == N(transfer)) {                                                   \
                action = N(onTransfer);                                                                              \
            }                                            \
            if ((code == TOKEN_CONTRACT && action == N(onTransfer)) || code == self && action != N(onTransfer)) {                               \
                TYPE thiscontract(self);                                                                             \
                switch (action)                                                                                      \
                {                                                                                                    \
                    EOSIO_API(TYPE, MEMBERS)                                                                         \
                }                                                                                                     \
            }                                                                                                        \
        }                                                                                                            \
    }

// generate .wasm and .wast file
EOSIO_WAST(pomelo, (onTransfer)(cancelbuy)(cancelsell)(buy)(sell))

// generate .abi file
// EOSIO_ABI(pomelo, (cancelbuy)(cancelsell)(buy)(sell))