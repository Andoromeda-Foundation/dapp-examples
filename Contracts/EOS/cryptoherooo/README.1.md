

cleos -u https://api-kylin.eosasia.one set contract tmonomonomon cryptoherooo -p tmonomonomon@active

eosiocpp -o cryptoherooo/cryptoherooo.wast cryptoherooo/cryptoherooo.cpp

cleos -u https://api-kylin.eosasia.one set contract tmonomonomon cryptoherooo -p tmonomonomon@active

cleos -u https://api-kylin.eosasia.one push action tmonomonomon init '["D533f24d6f28ddcef3f066474f7b8355383e485681ba8e793e037f5cf36e4883"]' -p tmonomonomon@active

cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "tmonomonomon", "0.0001 EOS", "draw" ]' -p minakokojima@active



cleos -u https://api-kylin.eosasia.one push action tmonomonomon reveal '["28349b1d4bcdc9905e4ef9719019e55743c84efa0c5e9a0b077f0b54fcd84905", "D533f24d6f28ddcef3f066474f7b8355383e485681ba8e793e037f5cf36e4883"]' -p tmonomonomon@active

cleos -u https://api-kylin.eosasia.one get table tmonomonomon tmonomonomon offer

cleos -u https://api-kylin.eosasia.one get table tmonomonomon tmonomonomon card



cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "tmonomonomon", "1 EOS", "draw" ]' -p minakokojima@active
