# Setup

## Unlock Wallet
cleos wallet unlock
PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u

## Setup Happyeosslot Contract
cleos create account eosio happyeosslot EOS6rRNxJWzdFtKH8MfsdvDci3vnC7QQhcRDA6a8PJV3tz8sEJYZz EOS76hCgV1mfHivJYb7wpbT4PNxxJricYdJCZVo8JtgNWyV4Td1h1
eosiocpp -o happyeosslot/happyeosslot.wast happyeosslot/happyeosslot.cpp
eosiocpp -g happyeosslot/happyeosslot.abi happyeosslot/happyeosslot.cpp
cleos -u https://api-kylin.eosasia.one set contract happyeosslot happyeosslot -p happyeosslot@active

cleos -u http://api-direct.eosasia.one push action happyeosslot init '["0196d5b5d9ec1bc78ba927d2db2cb327d836f002601c77bd8c3f144a07ddc737"]' -p happyeosslot@active



## Create minakokojima Account
cleos create account eosio tmonomonomon EOS6hcHfjnhdgPyRCMJneazKWRq6TjY5WyFpMfbVDrBX6e3uvUU4U EOS6hcHfjnhdgPyRCMJneazKWRq6TjY5WyFpMfbVDrBX6e3uvUU4U

## Issue some EOS Token
https://developers.eos.io/eosio-cpp/docs/quick-start-token
cleos create account eosio eosio.token EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
cleos set contract eosio.token build/contracts/eosio.token -p eosio.token
cleos push action eosio.token create '[ "eosio", "1000000000.0000 SYS"]' -p eosio.token
cleos push action eosio.token issue '[ "tmonomonomon", "100000000.0000 EOS", "memo" ]' -p eosio
cleos push action eosio.token issue '[ "tmonomonomon", "100000.0000 SYS", "memo" ]' -p eosio
cleos push action eosio.token issue '[ "happyeosslot", "100000.0000 SYS", "memo" ]' -p eosio
cleos push action eosio.token transfer '[ "tmonomonomon", "happyeosslot", "1.1000 SYS", "buy" ]' -p tmonomonomon@active



cleos -u http://api-direct.eosasia.one push action happyeosslot transfer '[ "eosotcbackup", "minakokojima", "0.0001 HPY", "" ]' -p eosotcbackup@active

cleos -u http://api-direct.eosasia.one push action happyeosslot transfer '[ "minakokojima", "eosotcbackup", "0.0001 HPY", "" ]' -p minakokojima@active

# Test
cleos push action eosio.token transfer '[ "tmonomonomon", "happyeosslot", "1.1000 EOS", "buy" ]' -p tmonomonomon@active
cleos push action eosio.token transfer '[ "tmonomonomon", "happyeosslot", "1.0000 EOS", "bet" ]' -p tmonomonomon@active
cleos push action happyeosslot transfer '[ "happyeosslot", "tmonomonomon", "1.0000 HPY", "got" ]' -p happyeosslot@active
cleos push action happyeosslot create '[ "happyeosslot", "1000000000.0000 HPZ" ]' -p happyeosslot@active
cleos push action happyeosslot issue '[ "tmonomonomon", "20.0000 HPY", "memo" ]' -p happyeosslot@active

cleos push action eosio.token transfer '[ "happyeosslot","tmonomonomon", "1.0000 EOS", "buy" ]' -p happyeosslot@active

cleos push action eosio.token transfer '[ "tmonomonomon","happyeosslot", "1.0000 EOS", "buy" ]' -p tmonomonomon@active

cleos push action happyeosslot ontransfer '[ "tmonomonomon", "happyeosslot", "11.0000 EOS", "buy" ]' -p tmonomonomon@active

cleos push action happyeosslot ontransfer '[ "happyeosslot", "tmonomonomon", "1.0000 EOS", "buy" ]' -p happyeosslot@active

## Query EOS
cleos get table eosio.token tmonomonomon accounts
cleos get table eosio.token happyeosslot accounts

## Query HPY
cleos get table happyeosslot tmonomonomon accounts
cleos -u https://api-kylin.eosasia.one get table happyeosslot eosotcbackup accounts

## Sell

void tradeableToken::sell(const account_name account, asset hpy) {


cleos -u http://api-direct.eosasia.one push action happyeosslot sell '["minakokojima", "33.6436 HPY"]' -p minakokojima@active




cleos -u https://api-kylin.eosasia.one push action happyeosslot sell '["minakokojima", "33.5000 HPY"]' -p minakokojima@active




cleos set account permission happyeosslot active '{"threshold": 1,"keys": [{"key": "EOS6rRNxJWzdFtKH8MfsdvDci3vnC7QQhcRDA6a8PJV3tz8sEJYZz","weight": 1}],"accounts": [{"permission":{"actor":"happyeosslot","permission":"eosio.code"},"weight":1}]}' owner -p happyeosslot

cleos push action eosio.token transfer '[ "happyeosslot","tmonomonomon", "450000.0001 EOS", "buy" ]' -p happyeosslot@active

cleos push action eosio.token transfer '[ "tmonomonomon","happyeosslot", "450000.0000 EOS", "buy" ]' -p tmonomonomon@active

cleos push action happyeosslot sell '[ "happyeosslot", "tmonomonomon", "1.0000 EOS", "buy" ]' -p happyeosslot@active

cleos push action happyeosslot transfer '[ "happyeosslot", "tmonomonomon", "1.0000 HPY", "got" ]' -p happyeosslot@active

cleos push action happyeosslot sell '["tmonomonomon", "10.0000 HPY"]' -p tmonomonomon@active



# Kylin

cleos -u https://api-kylin.eosasia.one push action happyeosdice init '["9993a3fa2e1f0d628a1bb9b27b7522ad29aa72881a1a36e39c2af301e86228d3"]' -p happyeosdice@active

cleos -u https://api-kylin.eosasia.one push action happyeosslot transfer '["cryptobuffff", "ceshiyongeos", "200.0000 HPY", "happy weekend"]' -p minakokojima@active

cleos -u https://api-kylin.eosasia.one push action happyeosslot issue '["tmonomonomon", "1.0000 HPY"]' -p happyeosslot@active

cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '["happyeosslot", "tmonomonomon", "100.0000 EOS", "buy"]' -p happyeosslot@active


cleos -u https://api-kylin.eosasia.one get table happyeosslot happyeosslot players

cleos -u https://api-kylin.eosasia.one get table eosio.token ceshiyongeos accounts

cleos -u https://api-kylin.eosasia.one get table happyeosslot dawnwalletes  accounts

cleos -u https://api-kylin.eosasia.one get table happyeosslot tmonomonomon accounts

cleos -u https://api-kylin.eosasia.one set contract happyeosslot happyeosslot -p happyeosslot@active

cleos -u https://api-kylin.eosasia.one set contract ceshiyongeos happyeosslot -p ceshiyongeos@active


cleos push action eosio.token transfer '["tmonomonomon", "happyeosslot", "1.0000 EOS"]' -p tmonomonomon@active



cleos -u https://api-kylin.eosasia.one get table ceshiyongeos tmonomonomon accounts

//
// 001.0000 EOS 009.9999 HPY 0.1 EOS/HPY
// 010.0000 EOS 099.9972 HPY
// 100.0000 EOS 999.7363 HPY

// 


cleos -u https://api-kylin.eosasia.one push action happyeosslot test '"tmonomonomon", ["33.6436 HPY"]' -p happyeosslot@active

cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '["tmonomonomon", "ceshiyongeos", "9.0000 EOS", "buy"]' -p tmonomonomon@active

cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '["tmonomonomon", "ceshiyongeos", "90.0000 EOS", "buy"]' -p tmonomonomon@active




cleos -u http://api-direct.eosasia.one push action happyeosslot transfer '["minakokojima", "cryptobuffff", "200.0000 HPY", "happy weekends"]' -p minakokojima@active