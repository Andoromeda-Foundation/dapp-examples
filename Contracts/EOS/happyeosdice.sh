/usr/local/eosio/bin/eosiocpp -o happyeosdice/happyeosdice.wast happyeosdice/happyeosdice.cpp

/usr/local/eosio/bin/eosiocpp -g happyeosdice/happyeosdice.abi happyeosdice/happyeosdice.cpp
cleos -u https://api-kylin.eosasia.one set contract happyeosdice happyeosdice -p happyeosdice@active




# cleos -u https://api-kylin.eosasia.one system newaccount --stake-net '5.0000 EOS' --stake-cpu '1.0000 EOS' --buy-ram '20.0000 EOS' minakokojima happyeosdice EOS5fY2dmpfXmzXN1DEJ6VfvhhCr55ZpE9MyTd9eX7Cw2EmzJcnFM EOS781pFN6TgUkuCDqmCdyxcp1cnJdtg3DYfajbAba6mvHvrFNuDy


