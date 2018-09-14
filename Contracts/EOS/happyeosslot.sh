
cleos wallet unlock --password PW5JaGpLtoM1vtD1WxiAC4RDsr82FRUczmKgocw1KJZqVahB4LZ1u
/usr/local/eosio/bin/eosiocpp -o happyeosslot/happyeosslot.wast happyeosslot/happyeosslot.cpp

# cleos -u http://api-direct.eosasia.one set contract happyeosslot happyeosslot -p happyeosslot@active
# cleos -u http://api-direct.eosasia.one get table happyeosslot happyeosslot offer

# cleos -u http://api-direct.eosasia.one set contract happyeosslot happyeosslot -p happyeosslot@active
# cleos -u https://api-kylin.eosasia.one push action happyeosslot init '["0196d5b5d9ec1bc78ba927d2db2cb327d836f002601c77bd8c3f144a07ddc737"]' -p happyeosslot@active

# cleos -u http://api-direct.eosasia.one push action happyeosslot test '["minakokojima", "1.0000 HPY"]' -p happyeosslot@active

# cleos -u http://api-direct.eosasia.one push action happyeosdice transfer '["minakokojima", "happyeosdice", "1.0000 EOS", "buy"]' -p happyeosdice@active


# /usr/local/eosio/bin/eosiocpp -o happyeosdice/happyeosdice.wast happyeosdice/happyeosdice.cpp

# /usr/local/eosio/bin/eosiocpp -g happyeosdice/happyeosdice.abi happyeosdice/happyeosdice.cpp
#cleos -u https://api-kylin.eosasia.one set contract happyeosdice happyeosdice -p happyeosdice@active
#cleos -u http://api-direct.eosasia.one push action eosio.token transfer '[ "minakokojima", "happyeosslot", "80.0000 EOS", "buy" ]' -p minakokojima@active
#cleos -u https://api-kylin.eosasia.one get table happyeosdice happyeosdice offer
#cleos -u https://api-kylin.eosasia.one get table happyeosdice minakokojima result


# cleos -u https://mainnet.genereos.io:443 push action happyeosslot init '["9993a3fa2e1f0d628a1bb9b27b7522ad29aa72881a1a36e39c2af301e86228d3"]' -p happyeosslot@active

# cleos -u https://api-kylin.eosasia.one system newaccount --stake-net '5.0000 EOS' --stake-cpu '1.0000 EOS' --buy-ram '20.0000 EOS' minakokojima happyeosdice EOS5fY2dmpfXmzXN1DEJ6VfvhhCr55ZpE9MyTd9eX7Cw2EmzJcnFM EOS781pFN6TgUkuCDqmCdyxcp1cnJdtg3DYfajbAba6mvHvrFNuDy




# cleos -u https://api-kylin.eosasia.one set account permission happyeosdice active '{"threshold": 1,"keys": [{"key": "EOS79mZ2pseEgc7cR4rPFoTUe3DBrLj8jmf3FGZqQCBwazUYioGZh","weight": 1}],"accounts": [{"permission":{"actor":"happyeosdice","permission":"eosio.code"},"weight":1}]}' owner -p happyeosdice
#