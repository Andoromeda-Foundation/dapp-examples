/usr/local/eosio/bin/eosiocpp -o happyeosslot/happyeosslot.wast happyeosslot/happyeosslot.cpp
cleos -u http://kylin.meet.one:8888 set contract happyeosslot happyeosslot -p happyeosslot@active

#cleos set contract happyeosslot happyeosslot -p happyeosslot@active
#cleos push action eosio.token transfer '[ "tmonomonomon","happyeosslot", "10.0000 EOS", "buy" ]' -p tmonomonomon@active




cleos -u http://api-kylin.eosasia.one push action happyeosslot test '["happyeosslot", "1.0000 EOS"]' -p happyeosslot@active
