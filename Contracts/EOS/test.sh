alias eosiocpp=/usr/local/eosio/bin/eosiocpp
alias cleos=/usr/local/eosio/bin/cleos
eosiocpp -o slot_machine/slot_machine.wast slot_machine/slot_machine.cpp
eosiocpp -g slot_machine/slot_machine.abi slot_machine/slot_machine.cpp
cleos set contract slot slot_machine -p slot@active

eosiocpp -o charger/charger.wast charger/charger.cpp
eosiocpp -g charger/charger.abi charger/charger.cpp


eosiocpp -o exchange/exchange.wast exchange/exchange.cpp

eosiocpp -o happyeosslot2.0/slot_machine.wast happyeosslot2.0/slot_machine.cpp
eosiocpp -g happyeosslot2.0/slot_machine.abi happyeosslot2.0/slot_machine.cpp


eosiocpp -o happyeosslot2.0/happyeosslot.wast happyeosslot2.0/happyeosslot.cpp
eosiocpp -g happyeosslot2.0/happyeosslot.abi happyeosslot2.0/happyeosslot.cpp





# 

cleos push action eosio.token issue '["alice", "1000.0000 SYS", "memo"]' -p eosio@active -d -j
cleos push action eosio.token issue '["alice", "1000.0000 EOS", "memo"]' -p eosio@active -d -j



cleos create account eosio happyeosslot EOS6rRNxJWzdFtKH8MfsdvDci3vnC7QQhcRDA6a8PJV3tz8sEJYZz EOS76hCgV1mfHivJYb7wpbT4PNxxJricYdJCZVo8JtgNWyV4Td1h1

eosiocpp -o happyeosslot/happyeosslot.wast happyeosslot/happyeosslot.cpp
eosiocpp -g happyeosslot/happyeosslot.abi happyeosslot/happyeosslot.cpp
cleos set contract happyeosslot happyeosslot -p happyeosslot@active


cleos push action happyeosslot f '["alice"]' -p alice

