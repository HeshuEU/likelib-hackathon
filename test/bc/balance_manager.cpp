#include <boost/test/unit_test.hpp>

#include "bc/balance_manager.hpp"

#include <random>
#include <thread>

std::map<bc::Address, bc::Balance> initMap()
{
    std::map<bc::Address, bc::Balance> init_map;
    init_map[bc::Address("qwerty")] = 1000;
    init_map[bc::Address("Troia")] = 185;
    init_map[bc::Address("okDe")] = 7;
    init_map[bc::Address("Andrei")] = 9999;
    init_map[bc::Address("back_door")] = 1'000'000;
    for(std::size_t i = 0; i < 10; i++){
        init_map[bc::Address("Person" + std::to_string(i))] = 113 * i;
    }
    return init_map;
}
std::map<bc::Address, bc::Balance> init_map = initMap();

BOOST_AUTO_TEST_CASE(balance_manager_constructor)
{
    bc::BalanceManager manager(init_map);

    BOOST_CHECK(manager.getBalance(bc::Address("qwerty")) == 1000);
    BOOST_CHECK(manager.getBalance(bc::Address("Andrei")) == 9999);
    BOOST_CHECK(manager.getBalance(bc::Address("back_door")) == 1'000'000);
    BOOST_CHECK(manager.getBalance(bc::Address("Ivan")) == 0);
}


BOOST_AUTO_TEST_CASE(balance_manager_check_transaction)
{
    bc::BalanceManager manager(init_map);

    bc::Transaction trans1(bc::Address("qwerty"), bc::Address("okDe"), 13, base::Time());
    bc::Transaction trans2(bc::Address("Andrei"), bc::Address("Troia"), 9999, base::Time());
    bc::Transaction trans3(bc::Address("back_door"), bc::Address("Ivan"), 1, base::Time());
    bc::Transaction trans4(bc::Address("okDe"), bc::Address("Ivan"), 19, base::Time());
    bc::Transaction trans5(bc::Address("Troia"), bc::Address("qwerty"), 190, base::Time());

    BOOST_CHECK(manager.checkTransaction(trans1));
    BOOST_CHECK(manager.checkTransaction(trans2));
    BOOST_CHECK(manager.checkTransaction(trans3));

    BOOST_CHECK(!manager.checkTransaction(trans4));
    BOOST_CHECK(!manager.checkTransaction(trans5));
}


BOOST_AUTO_TEST_CASE(balance_manager_update_transaction)
{
    bc::BalanceManager manager(init_map);
    manager.update(bc::Transaction(bc::Address("qwerty"), bc::Address("okDe"), 13, base::Time()));
    manager.update(bc::Transaction(bc::Address("Andrei"), bc::Address("Troia"), 11, base::Time()));
    manager.update(bc::Transaction(bc::Address("back_door"), bc::Address("Ivan"), 1, base::Time()));

    BOOST_CHECK(manager.getBalance(bc::Address("qwerty")) == 1000 - 13);
    BOOST_CHECK(manager.getBalance(bc::Address("Andrei")) == 9999 - 11);
    BOOST_CHECK(manager.getBalance(bc::Address("back_door")) == 1'000'000 - 1);
    BOOST_CHECK(manager.getBalance(bc::Address("okDe")) == 7 + 13);
    BOOST_CHECK(manager.getBalance(bc::Address("Ivan")) == 1);
}


void testUpdateTransaction(bc::BalanceManager& manager, std::vector<std::string>& names, std::vector<std::shared_mutex>& mutexes)
{
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<std::size_t> person_distribution(0, names.size() - 1);

    for(std::size_t i = 0; i < 25; i++) {
        std::size_t sender_pos;
        bool block = false;
        do {
            if(block) {
                mutexes[sender_pos].unlock();
            }
            sender_pos = person_distribution(generator);
            block = mutexes[sender_pos].try_lock();
        } while(manager.getBalance(bc::Address{names[sender_pos]}) == 0 || !block);

        std::size_t receiver_pos;
        block = false;
        do {
            if(block) {
                mutexes[receiver_pos].unlock();
            }
            receiver_pos = person_distribution(generator);
            block = mutexes[receiver_pos].try_lock();
        } while(!block);

        auto sender_tokens = manager.getBalance(bc::Address{names[sender_pos]});
        auto receiver_tokens = manager.getBalance(bc::Address{names[receiver_pos]});
        std::uniform_int_distribution<std::size_t> tokens_distribution(1, sender_tokens);
        auto transfer_tokens = tokens_distribution(generator);
        bc::Transaction transaction{bc::Address{names[sender_pos]}, bc::Address{names[receiver_pos]}, transfer_tokens, base::Time()};
        
        manager.update(transaction);

        BOOST_CHECK(manager.getBalance(bc::Address(names[sender_pos])) == sender_tokens - transfer_tokens &&
            manager.getBalance(bc::Address{names[receiver_pos]}) == receiver_tokens + transfer_tokens);

        mutexes[sender_pos].unlock();
        mutexes[receiver_pos].unlock();
    }
}


BOOST_AUTO_TEST_CASE(balance_manager_update_transaction_multithreads)
{
    bc::BalanceManager manager(init_map);
    std::vector<std::string> names{"qwerty", "Troia", "okDe", "Andrei", "Ivan", "Orland"};
    for(std::size_t i = 0; i < 10; i++){
        names.push_back("Person" + std::to_string(i));
    }
    std::vector<std::shared_mutex> mutexes(names.size());

    std::vector<std::thread> threads;
    for(std::size_t i = 0; i < 6; i++) {
        threads.push_back(std::thread(&testUpdateTransaction, std::ref(manager), std::ref(names), std::ref(mutexes)));
    }
    for(auto& thread: threads) {
        thread.join();
    }
}



