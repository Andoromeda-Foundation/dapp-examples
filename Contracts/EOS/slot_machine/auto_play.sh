#!/bin/bash
for (( ;1 < 100; ))
do
        sleep 360s
        cleos wallet unlock --password PW5K2J9uMBHR4nKbiRNY9iLkrYZc8QYzDqKpxdpPxpoZkVrramkdZ
        cleos -u http://api-direct.eosasia.one push action eosknightsio rebirth "[\"minakokojima\"]" -p minakokojima@active
        sleep 9s
done