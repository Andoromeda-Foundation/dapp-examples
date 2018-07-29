#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
using namespace eosio;

class counter_contract : public eosio::contract {
  public:
    counter_contract(account_name self) :eosio::contract(self){
        amount = 0;
    }
    using eosio::contract::contract;
    void add(account_name receiver) {
        todos.emplace(author, [&](auto& new_todo) {
            new_todo.id = id;
            new_todo.description = description;
            new_todo.completed = 0;
        });
        amount += 1;
        eosio::print("Now amount is %d", amount);
    }
  private:
  // @abi table todos i64
    struct todo {
      uint64_t id;

      uint64_t primary_key() const { return id; }
      EOSLIB_SERIALIZE(todo, (id))
    };

    typedef eosio::multi_index<N(todos), todo> todo_table;
    todo_table todos;
};

EOSIO_ABI( counter_contract, (add)(sub) )
