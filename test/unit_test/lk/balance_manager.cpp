#include <boost/test/unit_test.hpp>

#include "lk/balance_manager.hpp"

#include <random>
#include <thread>
#include <vector>

namespace
{
bc::Balance balance_each_person = 1000;

bc::Address qwerty = bc::Address::fromPublicKey(base::generateKeys().first);
bc::Address Troia = bc::Address::fromPublicKey(base::generateKeys().first);
bc::Address okDe = bc::Address::fromPublicKey(base::generateKeys().first);
bc::Address Andrei = bc::Address::fromPublicKey(base::generateKeys().first);
bc::Address back_door = bc::Address::fromPublicKey(base::generateKeys().first);
bc::Address Ivan = bc::Address::fromPublicKey(base::generateKeys().first);

std::pair<std::map<bc::Address, bc::Balance>, std::vector<bc::Address>> initMap()
{
    std::map<bc::Address, bc::Balance> init_map;
    std::vector<bc::Address> addresses;
    init_map[qwerty] = 1000;
    init_map[Troia] = 185;
    init_map[okDe] = 7;
    init_map[Andrei] = 9999;
    init_map[back_door] = 1'000'000;

    for(std::size_t i = 0; i < 90; i++) {
        addresses.push_back(bc::Address::fromPublicKey(base::generateKeys().first));
        init_map[addresses[i]] = balance_each_person;
    }
    return {init_map, addresses};
}

auto [init_map, addresses] = initMap();
} // namespace

BOOST_AUTO_TEST_CASE(balance_manager_constructor)
{
    lk::BalanceManager manager(init_map);

    BOOST_CHECK(manager.getBalance(qwerty) == 1000);
    BOOST_CHECK(manager.getBalance(Andrei) == 9999);
    BOOST_CHECK(manager.getBalance(back_door) == 1'000'000);
    BOOST_CHECK(manager.getBalance(Ivan) == 0);
}


BOOST_AUTO_TEST_CASE(balance_manager_check_transaction)
{
    lk::BalanceManager manager(init_map);

    bc::Transaction trans1(qwerty, okDe, 13, 0, base::Time());
    bc::Transaction trans2(Andrei, Troia, 9999, 0, base::Time());
    bc::Transaction trans3(back_door, Ivan, 1, 0, base::Time());
    bc::Transaction trans4(okDe, Ivan, 19, 0, base::Time());
    bc::Transaction trans5(Troia, qwerty, 190, 0, base::Time());

    BOOST_CHECK(manager.checkTransaction(trans1));
    BOOST_CHECK(manager.checkTransaction(trans2));
    BOOST_CHECK(manager.checkTransaction(trans3));

    BOOST_CHECK(!manager.checkTransaction(trans4));
    BOOST_CHECK(!manager.checkTransaction(trans5));
}


BOOST_AUTO_TEST_CASE(balance_manager_update_transaction)
{
    lk::BalanceManager manager(init_map);
    manager.update(bc::Transaction(qwerty, okDe, 13, 0, base::Time()));
    manager.update(bc::Transaction(Andrei, Troia, 11, 0, base::Time()));
    manager.update(bc::Transaction(back_door, Ivan, 1, 0, base::Time()));

    BOOST_CHECK(manager.getBalance(qwerty) == 1000 - 13);
    BOOST_CHECK(manager.getBalance(Andrei) == 9999 - 11);
    BOOST_CHECK(manager.getBalance(back_door) == 1'000'000 - 1);
    BOOST_CHECK(manager.getBalance(okDe) == 7 + 13);
    BOOST_CHECK(manager.getBalance(Ivan) == 1);
}


void testUpdateTransaction(lk::BalanceManager& manager, std::size_t sequence_number)
{
    constexpr std::size_t count_accounts = 9;
    constexpr std::size_t period = 2;
    constexpr bc::Balance transfer_tokens = 100;
    for(std::size_t i = 0; i < count_accounts * 100; i++) {
        std::size_t sender_pos = sequence_number * count_accounts + (i * period) % count_accounts;
        std::size_t receiver_pos = sequence_number * count_accounts + (i * period + period) % count_accounts;

        bc::Transaction transaction{addresses[sender_pos], addresses[receiver_pos], transfer_tokens, 0, base::Time()};

        manager.update(transaction);
    }

    bool res = true;
    for(std::size_t i = 0; i < count_accounts; i++) {
        res = res && (manager.getBalance(addresses[count_accounts * sequence_number + i]) == balance_each_person);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(balance_manager_update_transaction_multithreads)
{
    lk::BalanceManager manager(init_map);

    std::vector<std::thread> threads;
    for(std::size_t i = 0; i < 10; i++) {
        threads.emplace_back(&testUpdateTransaction, std::ref(manager), i);
    }
    for(auto& thread: threads) {
        thread.join();
    }
}


BOOST_AUTO_TEST_CASE(balance_manager_update_block)
{
    lk::BalanceManager manager(init_map);
    bc::TransactionsSet transaction_set;

    transaction_set.add(bc::Transaction(qwerty, okDe, 13, 0, base::Time()));
    transaction_set.add(bc::Transaction(Andrei, Troia, 11, 0, base::Time()));
    transaction_set.add(bc::Transaction(back_door, Ivan, 1, 0, base::Time()));
    bc::Block block(123, base::Sha256::compute(base::Bytes("")), std::move(transaction_set));
    manager.update(block);

    BOOST_CHECK(manager.getBalance(qwerty) == 1000 - 13);
    BOOST_CHECK(manager.getBalance(Andrei) == 9999 - 11);
    BOOST_CHECK(manager.getBalance(back_door) == 1'000'000 - 1);
    BOOST_CHECK(manager.getBalance(okDe) == 7 + 13);
    BOOST_CHECK(manager.getBalance(Ivan) == 1);
}


void testUpdateBlock(lk::BalanceManager& manager, std::size_t sequence_number)
{
    constexpr std::size_t count_accounts = 9;
    constexpr std::size_t period = 2;
    constexpr bc::Balance transfer_tokens = 100;
    constexpr std::size_t count_transaction = 10;

    for(std::size_t i = 0; i < count_accounts * 10; i++) {
        bc::TransactionsSet transaction_set;
        for(std::size_t j = 0; j < count_transaction; j++) {
            std::size_t sender_pos =
                sequence_number * count_accounts + (i * count_transaction * period + j * period) % count_accounts;
            std::size_t receiver_pos = sequence_number * count_accounts +
                (i * count_transaction * period + j * period + period) % count_accounts;
            transaction_set.add(
                bc::Transaction{addresses[sender_pos], addresses[receiver_pos], transfer_tokens, 0, base::Time()});
        }
        bc::Block block(1, base::Sha256::compute(base::Bytes("")), std::move(transaction_set));

        manager.update(block);
    }

    bool res = true;
    for(std::size_t i = 0; i < count_accounts; i++) {
        res = res && (manager.getBalance(addresses[count_accounts * sequence_number + i]) == balance_each_person);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(balance_manager_update_block_multithreads)
{
    lk::BalanceManager manager(init_map);

    std::vector<std::thread> threads;
    for(std::size_t i = 0; i < 10; i++) {
        threads.emplace_back(&testUpdateBlock, std::ref(manager), i);
    }
    for(auto& thread: threads) {
        thread.join();
    }
}
