#!/bin/bash
for (( ;1 < 100; ))
do
        sleep 360s
        cleos wallet unlock --password PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u
        cleos -u http://api-direct.eosasia.one push action eosknightsio rebirth "[\"minakokojima\"]" -p minakokojima@active
        sleep 9s
done
 

 cleos -u http://api-direct.eosasia.one  get table happyeosslot happyeosslot players