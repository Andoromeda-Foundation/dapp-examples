/usr/local/eosio/bin/eosiocpp -o happyeosslot/happyeosslot.wast happyeosslot/happyeosslot.cpp
# cleos -u https://mainnet.genereos.io:443 set contract happyeosslot happyeosslot -p happyeosslot@active
# cleos -u https://mainnet.genereos.io:443 push action happyeosslot test '["minakokojima", "1.0000 HPY"]' -p happyeosslot@active
# cleos -u https://mainnet.genereos.io:443 push action happyeosslot init '["9993a3fa2e1f0d628a1bb9b27b7522ad29aa72881a1a36e39c2af301e86228d3"]' -p happyeosslot@active
# cleos -u https://mainnet.genereos.io:443 push action eosio.token transfer '[ "minakokojima", "happyeosslot", "1.0000 EOS", "buy" ]' -p minakokojima@active

# cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "happyeosslot", "477.0000 EOS", "buy" ]' -p minakokojima@active
# cleos -u https://api-kylin.eosasia.one push action happyeosslot sell '["minakokojima", "8530.2762 HPY"]' -p minakokojima@active
# cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "happyeosslot", "999.0000 EOS", "buy" ]' -p minakokojima@active

# 9.9980 + 8530.2762

# 8541.0196
# 8541.0181
# 4774.1498 + 3766.8683

# cleos -u https://api-kylin.eosasia.one push action happyeosslot init '["9993a3fa2e1f0d628a1bb9b27b7522ad29aa72881a1a36e39c2af301e86228d3"]' -p happyeosslot@active

# cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "happyeosslot", "minakokojima", "4415.2873 EOS", "buy" ]' -p happyeosslot@active

# cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "happyeosslot", "1.0000 EOS", "buy" ]' -p minakokojima@active
# cleos -u https://api-kylin.eosasia.one push action happyeosslot test '["minakokojima", "1.0000 HPY"]' -p happyeosslot@active


# cleos -u https://api-kylin.eosasia.one push action happyeosslot init '["9993a3fa2e1f0d628a1bb9b27b7522ad29aa72881a1a36e39c2af301e86228d3"]' -p happyeosslot@active


# cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "tmonomonomon", "happyeosslot", "1.0000 EOS", "buy" ]' -p tmonomonomon@active
# cleos -u https://api-kylin.eosasia.one push action eosio.token transfer '[ "minakokojima", "happyeosslot", "1.0000 EOS", "buy" ]' -p minakokojima@active

#cleos -u https://mainnet.genereos.io:443 set contract happyeosslot happyeosslot -p happyeosslot@active

#cleos set contract happyeosslot happyeosslot -p happyeosslot@active
#cleos push action eosio.token transfer '[ "tmonomonomon","happyeosslot", "10.0000 EOS", "buy" ]' -p tmonomonomon@active

#cleos -u https://mainnet.genereos.io:443 push action happyeosslot test '["happyeosslot", "1.0000 EOS"]' -p happyeosslot@active
