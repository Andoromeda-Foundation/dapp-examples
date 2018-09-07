# Setup

## Unlock Wallet
cleos wallet unlock --password PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1

## Setup Happyeosslot Contract

### Kylin Account

{"msg": "succeeded", "keys": {"active_key": {"public": "EOS6TxQMRjQfdgvsy2XTb7AqtaUgEY8tSxQhsPXLhnXuyVhuLv6mW", "private": "5JdwtFFNqA73VSL59LU3M1kMt5gBotyUaq2C33yL7dLpV4SZfFM"}, "owner_key": {"public": "EOS8CSdxToZPbciA3Ydm4QTLWUvqehPvah7thK9FjRWmTA1r7fBPW", "private": "5JiMfDHmg8LaZ5kjkMhNN5aeRzEQ9rKxYJowRuRJymS6CsakALg"}}, "account": "happyeos21pt"}


cleos create account eosio happyeosslot EOS6rRNxJWzdFtKH8MfsdvDci3vnC7QQhcRDA6a8PJV3tz8sEJYZz EOS76hCgV1mfHivJYb7wpbT4PNxxJricYdJCZVo8JtgNWyV4Td1h1
eosiocpp -o happyeosslot/happyeosslot.wast happyeosslot/happyeosslot.cpp
eosiocpp -g happyeosslot/happyeosslot.abi happyeosslot/happyeosslot.cpp
cleos -u http://api-direct.eosasia.one set contract happyeosslot happyeosslot -p happyeosslot@active

