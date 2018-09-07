#include <eosiolib/crypto.h>
#include "eosaeroplane.hpp"

#include <cstdio>

 // @abi action
void aeroplane::newgame(const account_name msgsender, const uint64_t roundid, string playerstr) {
    //require_auth( msgsender );
    auto round = rounds.find(roundid);
    eosio_assert(round == rounds.end(), "round already created" ); 

    //init accounts.
    // char playerchar[playerstr.length()];
    // for(int i=0;i<playerstr.length();i++){
    //     playerchar[i] = playerstr[i];
    // }
    // string players[] = strtok(playerchar," ");
    std::vector<string> players = split(playerstr," ");

    std::vector<account_name> accounts;
    for(int i=0;i<players.size();i++){
        string k = players[i];
        accounts.push_back(eosio::string_to_name(k.c_str()));
    }

    //init etc arrays.
    std::vector<uint64_t> prepareds;
    for(int i=0;i<players.size();i++){ prepareds.push_back(0); }

    std::vector<uint64_t> poses;//pos:1001 = [1,1]; 121312 = [121,312]
    for(int i=0;i<players.size();i++){ poses.push_back(-1); }

    std::vector<uint64_t> winners;
    for(int i=0;i<players.size();i++){ winners.push_back(-1); }

    //insert.
    rounds.emplace(_self, [&](auto &round) {
        round.roundid = roundid;
        round.accounts = accounts;
        round.prepareds = prepareds;
        round.poses = poses;
        round.step_index = 0;
        round.winners = winners;
        round.is_started = false;
        round.is_ended = false;
    });
}

 // @abi action
void aeroplane::prepare(const account_name msgsender, const uint64_t roundid) {
    require_auth(msgsender);
    auto round = rounds.find(roundid);
    eosio_assert( round != rounds.end(), "no round" );
    eosio_assert( round->is_started != true, "is started" );
    for(int i=0;i<round->prepareds.size();i++){
        if (round->accounts[i] == msgsender){
            break;
        }
        eosio_assert(i != round->accounts.size() - 1, "no player" ); 
    }
    rounds.modify(round, msgsender, [&](auto &round) {
        for(int i=0;i<round.accounts.size();i++){
            if (round.accounts[i] == msgsender){
                round.prepareds[i] = 1;
                break;
            }
        }
    });
    for(int i=0;i<round->prepareds.size();i++){
        if (round->prepareds[i] == 0){
            break;
        }
        if(i == round->accounts.size() - 1) startgame(msgsender, roundid);
    }
}

 // @abi action
void aeroplane::step(const account_name msgsender, 
                    const uint64_t roundid, 
                    const uint64_t step_index) {
    require_auth(msgsender);
    auto round = rounds.find(roundid);
    eosio_assert( round != rounds.end(), "no round" );
    eosio_assert( round->is_started, "not started" );
    eosio_assert( step_index == round->step_index, "wrong step" );
    // judge action player
    uint64_t index = step_index % round->prepareds.size();
    eosio_assert( round->accounts[index] == msgsender, "wrong player" );

    auto random = random6();

    // do something..
    refreshround(msgsender, roundid);

    if (random == 6){
        std::vector<uint64_t> winners = round->winners;
        for(int i=0;i<winners.size();i++){ 
            if(winners[i] == -1){
                winners[i] = index;
                break;
            }
        }
        if(winners.size() == round->prepareds.size()){
            endgame(msgsender, roundid);
            return;
        } 
        rounds.modify(round, msgsender, [&](auto &round) {
            round.winners = winners;
        });
    }
    // calculate next player;
    for (int i=0;i<round->accounts.size();i++){ // == while(true)
        index = index + 1 % round->prepareds.size();
        auto step_next = step_index + 1;
        for(int i=0;i<round->winners.size();i++){
            if(index == round->winners[i]) continue;
        }
        rounds.modify(round, msgsender, [&](auto &round) {
            round.step_index = step_next;
        });
        break;
    }
}

 // @abi action
void aeroplane::endgame(const account_name msgsender, const uint64_t roundid) {
    auto round = rounds.find(roundid);
    eosio_assert( round != rounds.end(), "no round" );
    eosio_assert( round->is_started, "not started" );
    rounds.modify(round, msgsender, [&](auto &round) {
        round.is_ended = true;
    });
    // solve winner rewards..
}

 // @abi action
void aeroplane::refreshround(const account_name msgsender, const uint64_t roundid){
    // do something..
}

 // @abi action
uint64_t aeroplane::random6(){
    //return random 1-6..
    uint64_t r = 6;
    return r;
}

 // @abi action
void aeroplane::startgame(const account_name msgsender, const uint64_t roundid){
    //refresh map..
    auto round = rounds.find(roundid);
    rounds.modify(round, msgsender, [&](auto &round) {
        round.is_started = true;
    });
}

std::vector<std::string> aeroplane::split(const std::string& s, const std::string& delim)
{
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len)
    {
        int find_pos = s.find(delim, pos);
        if (find_pos < 0)
        {
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

// #define MY_EOSIO_ABI(TYPE, MEMBERS)                                                                                  \
//     extern "C"                                                                                                       \
//     {                                                                                                                \
//         void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                \
//         {                                                                                                            \
//                                                                                                                      \
//             auto self = receiver;                                                                                    \
//             if (action == N(onerror))                                                                                \
//             {                                                                                                        \
//                 eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
//             }                                                                                                        \
//             if (code == TOKEN_CONTRACT && action == N(transfer)) {                                                   \
//                 action = N(onTransfer);                                                                              \
//             }                                                                                                        \
//             if ((code == TOKEN_CONTRACT && action == N(onTransfer)) || code == self && action != N(onTransfer)) {                               \
//                 TYPE thiscontract(self);                                                                             \
//                 switch (action)                                                                                      \
//                 {                                                                                                    \
//                     EOSIO_API(TYPE, MEMBERS)                                                                         \
//                 }                                                                                                     \
//             }                                                                                                        \
//         }                                                                                                            \
//     }
// generate .wasm and .wast file
// MY_EOSIO_ABI(aeroplane, (newgame)(prepare)(step)(endgame)(refresh))

// generate .abi file
EOSIO_ABI(aeroplane, (newgame)(prepare)(step)(refreshround))
