# Setup

# Network
-u https://api-kylin.eosasia.one
-u http://api-direct.eosasia.one

## Unlock Wallet
cleos wallet unlock
PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u

## Setup Contract
eosiocpp -o cryptoherooo/cryptoherooo.wast cryptoherooo/cryptoherooo.cpp
eosiocpp -g cryptoherooo/cryptoherooo.abi cryptoherooo/cryptoherooo.cpp

## Test

cleos -u https://api-kylin.eosasia.one set contract cryptoherooo cryptoherooo -p cryptoherooo@active

cleos -u https://api-kylin.eosasia.one push action cryptoherooo init '["d533f24d6f28ddcef3f066474f7b8355383e485681ba8e793e037f5cf36e4883"]' -p cryptoherooo@active

cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "cryptoherooo", "0.0001 EOS", "draw" ]' -p alice

cleos -u https://api-kylin.eosasia.one push action slot reveal '["cryptoherooo", "28349b1d4bcdc9905e4ef9719019e55743c84efa0c5e9a0b077f0b54fcd84905", "D533f24d6f28ddcef3f066474f7b8355383e485681ba8e793e037f5cf36e4883"]' -p cryptoherooo@active
