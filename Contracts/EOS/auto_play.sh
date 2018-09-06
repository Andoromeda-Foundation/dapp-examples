#!/bin/bash
for (( ;1 < 100; ))
do
        cleos wallet unlock --password PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u
        cleos -u http://api-direct.eosasia.one push action eosio.token transfer "[\"tmonomonomon\", \"happyeosdice\", \"0.0100 EOS\" \"bet 150 ludufutemp minakokojima\"]" -p tmonomonomon@active
            sleep 3601s
done

