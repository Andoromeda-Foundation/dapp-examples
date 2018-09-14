#!/bin/bash
for (( ;1 < 100; ))
do
result=`cleos -u http://api-direct.eosasia.one get table happyeosslot happyeosslot offer`
if [ ${#result} -gt 35 ]; then
        sleep 1s
        cleos wallet unlock --password PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u
        last_seed=`cat rawseed.txt`
        echo "last Seed " $last_seed
        seed=`openssl rand 32 -hex`
        echo $seed
        # send last result and new seed.
        generated=`echo -n $seed | xxd -r -p | sha256sum -b | awk '{print $1}'`
        echo $generated
        echo `date` $seed " " $generated >> /root/seed.txt
        cleos -u http://api-direct.eosasia.one push action happyeosslot reveal "[\"happyeosslot\", \"$last_seed\", \"$generated\"]" -p happyeosslot@active
        cleos_result=$?
        if [ $cleos_result -eq 0 ]; then
                echo $seed > /root/rawseed.txt
        fi
fi
sleep 9s
done

cleos -u https://api-kylin.eosasia.one push action happyeosslot test '["ceshiyongeos", "1.0000 EOS"]' -p ceshiyongeos@active

cleos -u http://api-direct.eosasia.one  get table happyeosslot happyeosslot players
cleos -u http://api-direct.eosasia.one  set contract happyeosslot slot_machine  -p happyeosslot@active
eosiocpp -o slot_machine/slot_machine.wast slot_machine/slot_machine.cpp

eosiocpp -g slot_machine/slot_machine.abi slot_machine/slot_machine.cpp
