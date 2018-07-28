#ifndef ROSETTA_STONE_H_INCLUDED
#define ROSETTA_STONE_H_INCLUDED

#include <eosiolib/eosio.hpp>

class rosetta_stone : public eosio::contract {
   public:
      // rosetta_stone( account_name self ):contract(self){}

      struct stone {
         // static const uint16_t board_width = 3;
         // static const uint16_t board_height = board_width;
         stone() {
            initialize_stone();
         }
         // account_name          challenger;
         account_name          host; // like owner ?
         // account_name          turn; // = account name of host/ challenger
         // account_name          winner = N(none); // = none/ draw/ name of host/ name of challenger
         // std::vector<uint8_t>  board;

         // Initialize stone with empty cell
         void initialize_stone() {
            // board = std::vector<uint8_t>(board_width * board_height, 0);
         }

         // Reset stone
         //void reset_stone() {
         //   initialize_stone();
         //}

         // auto primary_key() const { return challenger; }
         // EOSLIB_SERIALIZE( game, (challenger)(host)(turn)(winner)(board))
      };

/* @abi action
      typedef eosio::multi_index< N(games), game> games;

      /// @abi action
      /// Create a new game
      void create(const account_name& challenger, const account_name& host);

      /// @abi action
      /// Restart a game
      /// @param by the account who wants to restart the game
      void restart(const account_name& challenger, const account_name& host, const account_name& by);

      /// @abi action
      /// Close an existing game, and remove it from storage
      void close(const account_name& challenger, const account_name& host);

      /// @abi action
      /// Make movement
      /// @param by the account who wants to make the move
      void move(const account_name& challenger, const account_name& host, const account_name& by, const uint16_t& row, const uint16_t& column);
*/
};

#endif // ROSETTA_STONE_H_INCLUDED
