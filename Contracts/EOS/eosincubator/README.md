# Setup

#
cleos -u https://api-kylin.eosasia.one get table pomelodex111 happyeosslot buyorder

cleos wallet unlock --password PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u

5JvpqdwPb1k8QQud1Yq7BGXJXrK1eLLyAJXEoX2mZQ4egimRGxe

/usr/local/eosio/bin/eosiocpp -o pomelo/pomelo.wast pomelo/pomelo.cpp


cleos -u https://api-kylin.eosasia.one set account permission eosotcbackup active '{"threshold": 1,"keys": [{"key": "EOS74dQ6NPbtytet1yWdm79bczHo9QA1WvP9Fn7cSpb6CakqwbrkZ","weight": 1}],"accounts": [{"permission":{"actor":"eosotcbackup","permission":"eosio.code"},"weight":1}]}' owner -p eosotcbackup


# cleos -u https://api-kylin.eosasia.one set contract pomelodex111 pomelo -p pomelodex111@active

# cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "happyeosslot", "1.0000 ", "buy" ]' -p minakokojima@active

# cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "pomelodex111", "0.3333 EOS", "buy,happyeosslot,HPY,0.6789" ]' -p minakokojima@active

# /usr/local/eosio/bin/eosiocpp -g pomelo/pomelo.abi pomelo/pomelo.cpp
# cleos -u https://api-kylin.eosasia.one set contract pomelodex111 pomelo -p pomelodex111@active
# cleos -u https://api-kylin.eosasia.one system newaccount --stake-net '5.0000 EOS' --stake-cpu '1.0000 EOS' --buy-ram '20.0000 EOS' minakokojima pomelodex111 EOS5fY2dmpfXmzXN1DEJ6VfvhhCr55ZpE9MyTd9eX7Cw2EmzJcnFM EOS781pFN6TgUkuCDqmCdyxcp1cnJdtg3DYfajbAba6mvHvrFNuDy


# Permission
# cleos -u https://api-kylin.eosasia.one set account permission eosotcbackup active '{"threshold": 1,"keys": [{"key": "EOS74dQ6NPbtytet1yWdm79bczHo9QA1WvP9Fn7cSpb6CakqwbrkZ","weight": 1}],"accounts": [{"permission":{"actor":"eosotcbackup","permission":"happyeosslot.code"},"weight":1}]}' owner -p eosotcbackup
