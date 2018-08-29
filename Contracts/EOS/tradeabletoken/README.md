# Setup

## Unlock Wallet
cleos wallet unlock
PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u

## Setup tradeabletoken Contract
cleos create account eosio tradeabletoken EOS6rRNxJWzdFtKH8MfsdvDci3vnC7QQhcRDA6a8PJV3tz8sEJYZz EOS76hCgV1mfHivJYb7wpbT4PNxxJricYdJCZVo8JtgNWyV4Td1h1
eosiocpp -o tradeabletoken/tradeabletoken.wast tradeabletoken/tradeabletoken.cpp
eosiocpp -g tradeabletoken/tradeabletoken.abi tradeabletoken/tradeabletoken.cpp

cleos -u https://api-kylin.eosasia.one set contract tokentesteos tradeabletoken -p tokentesteos@active

cleos -u https://api-kylin.eosasia.one set contract xinzhanghao1 tradeabletoken -p xinzhanghao1@active



cleos -u https://api-kylin.eosasia.one set account permission tokentesteos active '{"threshold": 1,"keys": [{"key": "EOS5NVHrr6zANRxrZQ3JujGbNGyrKXurQTzke1ThrcUKvGKkA1SWv","weight": 1}],"accounts": [{"permission":{"actor":"tokentesteos","permission":"eosio.code"},"weight":1}]}' owner -p tokentesteos


cleos -u https://api-kylin.eosasia.one push action xinzhanghao1 init '[]' -p xinzhanghao1@active