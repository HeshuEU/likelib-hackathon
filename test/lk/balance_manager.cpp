#include <boost/test/unit_test.hpp>

#include "lk/balance_manager.hpp"

#include <random>
#include <thread>

namespace
{
std::map<bc::Address, bc::Balance> initMap()
{
    std::map<bc::Address, bc::Balance> init_map;
    init_map[bc::Address("qwerty")] = 1000;
    init_map[bc::Address("Troia")] = 185;
    init_map[bc::Address("okDe")] = 7;
    init_map[bc::Address("Andrei")] = 9999;
    init_map[bc::Address("back_door")] = 1'000'000;

    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<bc::Balance> tokens_distribution(0, 10000);
    for(std::size_t i = 0; i < 30; i++) {
        init_map[bc::Address("Person" + std::to_string(i))] = tokens_distribution(generator);
    }
    return init_map;
}
std::map<bc::Address, bc::Balance> init_map = initMap();
} // namespace

BOOST_AUTO_TEST_CASE(balance_manager_constructor)
{
    lk::BalanceManager manager(init_map);

    BOOST_CHECK(manager.getBalance(bc::Address("qwerty")) == 1000);
    BOOST_CHECK(manager.getBalance(bc::Address("Andrei")) == 9999);
    BOOST_CHECK(manager.getBalance(bc::Address("back_door")) == 1'000'000);
    BOOST_CHECK(manager.getBalance(bc::Address("Ivan")) == 0);
}


BOOST_AUTO_TEST_CASE(balance_manager_check_transaction)
{
    lk::BalanceManager manager(init_map);

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
    lk::BalanceManager manager(init_map);
    manager.update(bc::Transaction(bc::Address("qwerty"), bc::Address("okDe"), 13, base::Time()));
    manager.update(bc::Transaction(bc::Address("Andrei"), bc::Address("Troia"), 11, base::Time()));
    manager.update(bc::Transaction(bc::Address("back_door"), bc::Address("Ivan"), 1, base::Time()));

    BOOST_CHECK(manager.getBalance(bc::Address("qwerty")) == 1000 - 13);
    BOOST_CHECK(manager.getBalance(bc::Address("Andrei")) == 9999 - 11);
    BOOST_CHECK(manager.getBalance(bc::Address("back_door")) == 1'000'000 - 1);
    BOOST_CHECK(manager.getBalance(bc::Address("okDe")) == 7 + 13);
    BOOST_CHECK(manager.getBalance(bc::Address("Ivan")) == 1);
}


void testUpdateTransaction(
    lk::BalanceManager& manager, std::vector<std::string>& names, std::vector<std::shared_mutex>& mutexes)
{
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<std::size_t> person_distribution(0, names.size() - 1);

    for(std::size_t i = 0; i < 25; i++) {

        std::size_t sender_pos;
        bool lock = false;
        do {
            if(lock) {
                mutexes[sender_pos].unlock();
            }
            sender_pos = person_distribution(generator);
            lock = mutexes[sender_pos].try_lock();
        } while(manager.getBalance(bc::Address{names[sender_pos]}) == 0 || !lock);

        std::size_t receiver_pos;
        lock = false;
        do {
            if(lock) {
                mutexes[receiver_pos].unlock();
            }
            receiver_pos = person_distribution(generator);
            lock = mutexes[receiver_pos].try_lock();
        } while(!lock);

        auto sender_tokens = manager.getBalance(bc::Address{names[sender_pos]});
        auto receiver_tokens = manager.getBalance(bc::Address{names[receiver_pos]});
        std::uniform_int_distribution<bc::Balance> tokens_distribution(1, sender_tokens);
        auto transfer_tokens = tokens_distribution(generator);
        bc::Transaction transaction{
            bc::Address{names[sender_pos]}, bc::Address{names[receiver_pos]}, transfer_tokens, base::Time()};

        manager.update(transaction);

        BOOST_CHECK(manager.getBalance(bc::Address(names[sender_pos])) == sender_tokens - transfer_tokens &&
            manager.getBalance(bc::Address{names[receiver_pos]}) == receiver_tokens + transfer_tokens);

        mutexes[sender_pos].unlock();
        mutexes[receiver_pos].unlock();
    }
}


BOOST_AUTO_TEST_CASE(balance_manager_update_transaction_multithreads)
{
    lk::BalanceManager manager(init_map);
    std::vector<std::string> names{"qwerty", "Troia", "okDe", "Andrei", "Ivan", "Orland"};
    for(std::size_t i = 0; i < init_map.size() - 5; i++) {
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


BOOST_AUTO_TEST_CASE(balance_manager_update_block)
{
    lk::BalanceManager manager(init_map);
    bc::TransactionsSet transaction_set;
    
    transaction_set.add(bc::Transaction(bc::Address("qwerty"), bc::Address("okDe"), 13, base::Time()));
    transaction_set.add(bc::Transaction(bc::Address("Andrei"), bc::Address("Troia"), 11, base::Time()));
    transaction_set.add(bc::Transaction(bc::Address("back_door"), bc::Address("Ivan"), 1, base::Time()));
    bc::Block block(base::Sha256::compute(base::Bytes("")), std::move(transaction_set));
    manager.update(block);

    BOOST_CHECK(manager.getBalance(bc::Address("qwerty")) == 1000 - 13);
    BOOST_CHECK(manager.getBalance(bc::Address("Andrei")) == 9999 - 11);
    BOOST_CHECK(manager.getBalance(bc::Address("back_door")) == 1'000'000 - 1);
    BOOST_CHECK(manager.getBalance(bc::Address("okDe")) == 7 + 13);
    BOOST_CHECK(manager.getBalance(bc::Address("Ivan")) == 1);
}


void testUpdateBlock(
    lk::BalanceManager& manager, std::vector<std::string>& names, std::vector<std::shared_mutex>& mutexes)
{
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<std::size_t> person_distribution(0, names.size() - 1);
    std::uniform_int_distribution<std::size_t> transaction_distribution(1, 5);

    for(std::size_t i = 0; i < 10; i++) {

        std::size_t count_transactions = transaction_distribution(generator);

        std::vector<std::size_t> senders;
        bool lock = false;
        while(senders.size() != count_transactions) {
            std::size_t sender_pos;
            sender_pos = person_distribution(generator);
            lock = mutexes[sender_pos].try_lock();
            if(manager.getBalance(bc::Address{names[sender_pos]}) != 0 && lock) {
                senders.push_back(sender_pos);
            }
            else if(lock) {
                mutexes[sender_pos].unlock();
            }
        }

        std::vector<std::size_t> receivers;
        lock = false;
        while(receivers.size() != count_transactions) {
            std::size_t receiver_pos;
            receiver_pos = person_distribution(generator);
            lock = mutexes[receiver_pos].try_lock();
            if(lock) {
                receivers.push_back(receiver_pos);
            }
            else if(lock) {
                mutexes[receiver_pos].unlock();
            }
        }

        std::vector<std::tuple<bc::Balance, bc::Balance, bc::Balance>> test_info;
        bc::TransactionsSet transaction_set;
        for(std::size_t i = 0; i < count_transactions; i++) {
            auto sender_tokens = manager.getBalance(bc::Address{names[senders[i]]});
            auto receiver_tokens = manager.getBalance(bc::Address{names[receivers[i]]});
            std::uniform_int_distribution<bc::Balance> tokens_distribution(1, sender_tokens);
            auto transfer_tokens = tokens_distribution(generator);
            test_info.push_back(std::make_tuple(sender_tokens, receiver_tokens, transfer_tokens));
            transaction_set.add(bc::Transaction{
                bc::Address{names[senders[i]]}, bc::Address{names[receivers[i]]}, transfer_tokens, base::Time()});
        }
        bc::Block block(base::Sha256::compute(base::Bytes("")), std::move(transaction_set));

        manager.update(block);

        bool is_ok = true;
        for(std::size_t i = 0; i < count_transactions; i++) {
            is_ok = is_ok &&
                (manager.getBalance(bc::Address(names[senders[i]])) ==
                    std::get<0>(test_info[i]) - std::get<2>(test_info[i]));
            is_ok = is_ok &&
                (manager.getBalance(bc::Address(names[receivers[i]])) ==
                    std::get<1>(test_info[i]) + std::get<2>(test_info[i]));
        }
        BOOST_CHECK(is_ok);

        for(std::size_t i = 0; i < count_transactions; i++) {
            mutexes[senders[i]].unlock();
            mutexes[receivers[i]].unlock();
        }
    }
}


BOOST_AUTO_TEST_CASE(balance_manager_update_block_multithreads)
{
    lk::BalanceManager manager(init_map);
    std::vector<std::string> names{"qwerty", "Troia", "okDe", "Andrei", "Ivan", "Orland"};
    for(std::size_t i = 0; i < init_map.size() - 5; i++) {
        names.push_back("Person" + std::to_string(i));
    }
    std::vector<std::shared_mutex> mutexes(names.size());

    std::vector<std::thread> threads;
    for(std::size_t i = 0; i < 6; i++) {
        threads.push_back(std::thread(&testUpdateBlock, std::ref(manager), std::ref(names), std::ref(mutexes)));
    }
    for(auto& thread: threads) {
        thread.join();
    }
}
