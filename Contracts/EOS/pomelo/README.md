# Setup

## Unlock Wallet
cleos wallet unlock
PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u

## Setup pomelo Contract
cleos create account eosio pomelo EOS6rRNxJWzdFtKH8MfsdvDci3vnC7QQhcRDA6a8PJV3tz8sEJYZz EOS76hCgV1mfHivJYb7wpbT4PNxxJricYdJCZVo8JtgNWyV4Td1h1
eosiocpp -o pomelo/pomelo.wast pomelo/pomelo.cpp
eosiocpp -g pomelo/pomelo.abi pomelo/pomelo.cpp
cleos set contract pomelo pomelo -p pomelo@active


cleos -u http://api-direct.eosasia.one push action pomelo init '["0196d5b5d9ec1bc78ba927d2db2cb327d836f002601c77bd8c3f144a07ddc737"]' -p pomelo@active



## Create minakokojima Account
cleos create account eosio tmonomonomon EOS6hcHfjnhdgPyRCMJneazKWRq6TjY5WyFpMfbVDrBX6e3uvUU4U EOS6hcHfjnhdgPyRCMJneazKWRq6TjY5WyFpMfbVDrBX6e3uvUU4U

## Issue some EOS Token
https://developers.eos.io/eosio-cpp/docs/quick-start-token
cleos create account eosio eosio.token EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
cleos set contract eosio.token build/contracts/eosio.token -p eosio.token
cleos push action eosio.token create '[ "eosio", "1000000000.0000 SYS"]' -p eosio.token
cleos push action eosio.token issue '[ "tmonomonomon", "100000000.0000 EOS", "memo" ]' -p eosio
cleos push action eosio.token issue '[ "tmonomonomon", "100000.0000 SYS", "memo" ]' -p eosio
cleos push action eosio.token issue '[ "pomelo", "100000.0000 SYS", "memo" ]' -p eosio
cleos push action eosio.token transfer '[ "tmonomonomon", "pomelo", "1.1000 SYS", "buy" ]' -p tmonomonomon@active

# Test
cleos push action eosio.token transfer '[ "tmonomonomon", "pomelo", "1.1000 EOS", "buy" ]' -p tmonomonomon@active
cleos push action eosio.token transfer '[ "tmonomonomon", "pomelo", "1.0000 EOS", "bet" ]' -p tmonomonomon@active
cleos push action pomelo transfer '[ "pomelo", "tmonomonomon", "1.0000 HPY", "got" ]' -p pomelo@active
cleos push action pomelo create '[ "pomelo", "1000000000.0000 HPZ" ]' -p pomelo@active
cleos push action pomelo issue '[ "tmonomonomon", "20.0000 HPY", "memo" ]' -p pomelo@active

cleos push action eosio.token transfer '[ "pomelo","tmonomonomon", "1.0000 EOS", "buy" ]' -p pomelo@active

cleos push action eosio.token transfer '[ "tmonomonomon","pomelo", "1.0000 EOS", "buy" ]' -p tmonomonomon@active

cleos push action pomelo ontransfer '[ "tmonomonomon", "pomelo", "11.0000 EOS", "buy" ]' -p tmonomonomon@active

cleos push action pomelo ontransfer '[ "pomelo", "tmonomonomon", "1.0000 EOS", "buy" ]' -p pomelo@active

## Query EOS
cleos get table eosio.token tmonomonomon accounts
cleos get table eosio.token pomelo accounts

## Query HPY
cleos get table pomelo tmonomonomon accounts
cleos -u http://api-direct.eosasia.one get table pomelo pomelo accounts

## Sell

void tradeableToken::sell(const account_name account, asset hpy) {


cleos -u http://api-direct.eosasia.one push action pomelo sell '["minakokojima", "33.6436 HPY"]' -p minakokojima@active




cleos -u https://api-kylin.eosasia.one push action pomelo sell '["minakokojima", "33.5000 HPY"]' -p minakokojima@active




cleos set account permission pomelo active '{"threshold": 1,"keys": [{"key": "EOS6rRNxJWzdFtKH8MfsdvDci3vnC7QQhcRDA6a8PJV3tz8sEJYZz","weight": 1}],"accounts": [{"permission":{"actor":"pomelo","permission":"eosio.code"},"weight":1}]}' owner -p pomelo

cleos push action eosio.token transfer '[ "pomelo","tmonomonomon", "450000.0001 EOS", "buy" ]' -p pomelo@active

cleos push action eosio.token transfer '[ "tmonomonomon","pomelo", "450000.0000 EOS", "buy" ]' -p tmonomonomon@active

cleos push action pomelo sell '[ "pomelo", "tmonomonomon", "1.0000 EOS", "buy" ]' -p pomelo@active

cleos push action pomelo transfer '[ "pomelo", "tmonomonomon", "1.0000 HPY", "got" ]' -p pomelo@active

cleos push action pomelo sell '["tmonomonomon", "10.0000 HPY"]' -p tmonomonomon@active



# Kylin

cleos -u https://api-kylin.eosasia.one push action ceshiyongeos init '["ceshiyongeos", "d533f24d6f28ddcef3f066474f7b8355383e485681ba8e793e037f5cf36e4883"]' -p ceshiyongeos@active

cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '["tmonomonomon", "ceshiyongeos", "1.0000 EOS", "buy"]' -p tmonomonomon@active

cleos -u https://api-kylin.eosasia.one push action pomelo issue '["tmonomonomon", "1.0000 HPY"]' -p pomelo@active

cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '["pomelo", "tmonomonomon", "100.0000 EOS", "buy"]' -p pomelo@active


cleos -u https://api-kylin.eosasia.one get table pomelo pomelo players

cleos -u https://api-kylin.eosasia.one get table eosio.token ceshiyongeos accounts

cleos -u https://api-kylin.eosasia.one get table pomelo dawnwalletes  accounts

cleos -u https://api-kylin.eosasia.one get table pomelo tmonomonomon accounts

cleos -u https://api-kylin.eosasia.one set contract pomelo pomelo -p pomelo@active

cleos -u https://api-kylin.eosasia.one set contract ceshiyongeos pomelo -p ceshiyongeos@active


cleos push action eosio.token transfer '["tmonomonomon", "pomelo", "1.0000 EOS"]' -p tmonomonomon@active



cleos -u https://api-kylin.eosasia.one get table ceshiyongeos tmonomonomon accounts

//
// 001.0000 EOS 009.9999 HPY 0.1 EOS/HPY
// 010.0000 EOS 099.9972 HPY
// 100.0000 EOS 999.7363 HPY

// 



cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '["tmonomonomon", "ceshiyongeos", "9.0000 EOS", "buy"]' -p tmonomonomon@active

cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '["tmonomonomon", "ceshiyongeos", "90.0000 EOS", "buy"]' -p tmonomonomon@active