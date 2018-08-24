# Setup

## Setup Happyeosslot Contract
cleos create account eosio happyeosslot EOS6rRNxJWzdFtKH8MfsdvDci3vnC7QQhcRDA6a8PJV3tz8sEJYZz EOS76hCgV1mfHivJYb7wpbT4PNxxJricYdJCZVo8JtgNWyV4Td1h1
eosiocpp -o happyeosslot/happyeosslot.wast happyeosslot/happyeosslot.cpp
eosiocpp -g happyeosslot/happyeosslot.abi happyeosslot/happyeosslot.cpp
cleos set contract happyeosslot happyeosslot -p happyeosslot@active

## Create minakokojima Account
cleos create account eosio minakokojima EOS6qw7FaYr4VmUXczsGoVNRsXxopfVHLVXzLwZbXpADnASTmpBAu 
EOS6qw7FaYr4VmUXczsGoVNRsXxopfVHLVXzLwZbXpADnASTmpBAu

## Issue some EOS Token
https://developers.eos.io/eosio-cpp/docs/quick-start-token
cleos create account eosio eosio.token EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
cleos set contract eosio.token build/contracts/eosio.token -p eosio.token
cleos push action eosio.token create '[ "eosio", "1000000000.0000 "]' -p eosio.token
cleos push action eosio.token issue '[ "minakokojima", "100000.0000 EOS", "memo" ]' -p eosio

# Test