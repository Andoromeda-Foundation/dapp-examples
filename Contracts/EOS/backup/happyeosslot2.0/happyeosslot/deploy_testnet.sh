cleos -u https://api-kylin.eosasia.one set contract happyeosslot ../happyeosslot -p happyeosslot@active
cleos -u https://api-kylin.eosasia.one push action happyeosslot init '["0196d5b5d9ec1bc78ba927d2db2cb327d836f002601c77bd8c3f144a07ddc737"]' -p happyeosslot@active
cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "happyeosslot", "1.0000 EOS", "buy" ]' -p minakokojima@active
cleos -u https://api-kylin.eosasia.one push action happyeosslot sell '[ "minakokojima", "1.0000 HPY" ]' -p minakokojima@active


cleos -u https://api-kylin.eosasia.one set account permission happyeosslot active '{"threshold": 1,"keys": [{"key": "EOS78vWucrUNYkqSGbSbKCP6ugFiVD7wDrYTpb1NgCxfb2cnZ2U3q","weight": 1}],"accounts": [{"permission":{"actor":"happyeosslot","permission":"eosio.code"},"weight":1}]}' owner -p happyeosslot@active
