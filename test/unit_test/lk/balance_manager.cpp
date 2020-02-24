//#include <boost/test/unit_test.hpp>
//
//#include "lk/balance_manager.hpp"
//
//#include <random>
//#include <thread>
//
// namespace
//{
// bc::Balance balance_each_person = 1000;
//
// std::map<bc::Address, bc::Balance> initMap()
//{
//    std::map<bc::Address, bc::Balance> init_map;
//    init_map[bc::Add"qwerty").toHex()] = 1000;
//    init_map[base::Bytes("Troia").toHex()] = 185;
//    init_map[base::Bytes("okDe").toHex()] = 7;
//    init_map[base::Bytes("Andrei").toHex()] = 9999;
//    init_map[base::Bytes("back_door").toHex()] = 1'000'000;
//
//    for(std::size_t i = 0; i < 90; i++) {
//        init_map[base::Bytes(std::to_string(i)).toHex()] = balance_each_person;
//    }
//    return init_map;
//}
// std::map<bc::Address, bc::Balance> init_map = initMap();
//} // namespace
//
// BOOST_AUTO_TEST_CASE(balance_manager_constructor)
//{
//    lk::AccountManager manager(init_map);
//
//    BOOST_CHECK(manager.getBalance(base::Bytes("qwerty").toHex()) == 1000);
//    BOOST_CHECK(manager.getBalance(base::Bytes("Andrei").toHex()) == 9999);
//    BOOST_CHECK(manager.getBalance(base::Bytes("back_door").toHex()) == 1'000'000);
//    BOOST_CHECK(manager.getBalance(base::Bytes("Ivan").toHex()) == 0);
//}
//
//
// BOOST_AUTO_TEST_CASE(balance_manager_check_transaction)
//{
//    lk::AccountManager manager(init_map);
//
//    bc::Transaction trans1(base::Bytes("qwerty").toHex(), base::Bytes("okDe").toHex(), 13, base::Time());
//    bc::Transaction trans2(base::Bytes("Andrei").toHex(), base::Bytes("Troia").toHex(), 9999, base::Time());
//    bc::Transaction trans3(base::Bytes("back_door").toHex(), base::Bytes("Ivan").toHex(), 1, base::Time());
//    bc::Transaction trans4(base::Bytes("okDe").toHex(), base::Bytes("Ivan").toHex(), 19, base::Time());
//    bc::Transaction trans5(base::Bytes("Troia").toHex(), base::Bytes("qwerty").toHex(), 190, base::Time());
//
//    BOOST_CHECK(manager.checkTransaction(trans1));
//    BOOST_CHECK(manager.checkTransaction(trans2));
//    BOOST_CHECK(manager.checkTransaction(trans3));
//
//    BOOST_CHECK(!manager.checkTransaction(trans4));
//    BOOST_CHECK(!manager.checkTransaction(trans5));
//}
//
//
// BOOST_AUTO_TEST_CASE(balance_manager_update_transaction)
//{
//    lk::AccountManager manager(init_map);
//    manager.update(bc::Transaction(base::Bytes("qwerty").toHex(), base::Bytes("okDe").toHex(), 13, base::Time()));
//    manager.update(bc::Transaction(base::Bytes("Andrei").toHex(), base::Bytes("Troia").toHex(), 11, base::Time()));
//    manager.update(bc::Transaction(base::Bytes("back_door").toHex(), base::Bytes("Ivan").toHex(), 1, base::Time()));
//
//    BOOST_CHECK(manager.getBalance(base::Bytes("qwerty").toHex()) == 1000 - 13);
//    BOOST_CHECK(manager.getBalance(base::Bytes("Andrei").toHex()) == 9999 - 11);
//    BOOST_CHECK(manager.getBalance(base::Bytes("back_door").toHex()) == 1'000'000 - 1);
//    BOOST_CHECK(manager.getBalance(base::Bytes("okDe").toHex()) == 7 + 13);
//    BOOST_CHECK(manager.getBalance(base::Bytes("Ivan").toHex()) == 1);
//}
//
//
// void testUpdateTransaction(lk::AccountManager& manager, std::size_t sequence_number)
//{
//    constexpr std::size_t count_accounts = 9;
//    constexpr std::size_t period = 2;
//    constexpr bc::Balance transfer_tokens = 100;
//    for(std::size_t i = 0; i < count_accounts * 100; i++) {
//        std::size_t sender_pos = sequence_number * count_accounts + (i * period) % count_accounts;
//        std::size_t receiver_pos = sequence_number * count_accounts + (i * period + period) % count_accounts;
//
//        bc::Transaction transaction{base::Bytes{std::to_string(sender_pos)}.toHex(),
//            base::Bytes{std::to_string(receiver_pos)}.toHex(), transfer_tokens, base::Time()};
//
//        manager.update(transaction);
//    }
//
//    bool res = true;
//    for(std::size_t i = 0; i < count_accounts; i++) {
//        res = res &&
//            (manager.getBalance(base::Bytes(std::to_string(count_accounts * sequence_number + i)).toHex()) ==
//                balance_each_person);
//    }
//    BOOST_CHECK(res);
//}
//
//
// BOOST_AUTO_TEST_CASE(balance_manager_update_transaction_multithreads)
//{
//    lk::AccountManager manager(init_map);
//
//    std::vector<std::thread> threads;
//    for(std::size_t i = 0; i < 10; i++) {
//        threads.emplace_back(&testUpdateTransaction, std::ref(manager), i);
//    }
//    for(auto& thread: threads) {
//        thread.join();
//    }
//}
//
//
// BOOST_AUTO_TEST_CASE(balance_manager_update_block)
//{
//    lk::AccountManager manager(init_map);
//    bc::TransactionsSet transaction_set;
//
//    transaction_set.add(bc::Transaction(base::Bytes("qwerty").toHex(), base::Bytes("okDe").toHex(), 13,
//    base::Time())); transaction_set.add(bc::Transaction(base::Bytes("Andrei").toHex(), base::Bytes("Troia").toHex(),
//    11, base::Time())); transaction_set.add(
//        bc::Transaction(base::Bytes("back_door").toHex(), base::Bytes("Ivan").toHex(), 1, base::Time()));
//    bc::Block block(123, base::Sha256::compute(base::Bytes("")), std::move(transaction_set));
//    manager.update(block);
//
//    BOOST_CHECK(manager.getBalance(base::Bytes("qwerty").toHex()) == 1000 - 13);
//    BOOST_CHECK(manager.getBalance(base::Bytes("Andrei").toHex()) == 9999 - 11);
//    BOOST_CHECK(manager.getBalance(base::Bytes("back_door").toHex()) == 1'000'000 - 1);
//    BOOST_CHECK(manager.getBalance(base::Bytes("okDe").toHex()) == 7 + 13);
//    BOOST_CHECK(manager.getBalance(base::Bytes("Ivan").toHex()) == 1);
//}
//
//
// void testUpdateBlock(lk::AccountManager& manager, std::size_t sequence_number)
//{
//    constexpr std::size_t count_accounts = 9;
//    constexpr std::size_t period = 2;
//    constexpr bc::Balance transfer_tokens = 100;
//    constexpr std::size_t count_transaction = 10;
//
//    for(std::size_t i = 0; i < count_accounts * 10; i++) {
//        bc::TransactionsSet transaction_set;
//        for(std::size_t j = 0; j < count_transaction; j++) {
//            std::size_t sender_pos =
//                sequence_number * count_accounts + (i * count_transaction * period + j * period) % count_accounts;
//            std::size_t receiver_pos = sequence_number * count_accounts +
//                (i * count_transaction * period + j * period + period) % count_accounts;
//            transaction_set.add(bc::Transaction{base::Bytes{std::to_string(sender_pos)}.toHex(),
//                base::Bytes{std::to_string(receiver_pos)}.toHex(), transfer_tokens, base::Time()});
//        }
//        bc::Block block(1, base::Sha256::compute(base::Bytes("")), std::move(transaction_set));
//
//        manager.update(block);
//    }
//
//    bool res = true;
//    for(std::size_t i = 0; i < count_accounts; i++) {
//        res = res &&
//            (manager.getBalance(base::Bytes(std::to_string(count_accounts * sequence_number + i)).toHex()) ==
//                balance_each_person);
//    }
//    BOOST_CHECK(res);
//}
//
//
// BOOST_AUTO_TEST_CASE(balance_manager_update_block_multithreads)
//{
//    lk::AccountManager manager(init_map);
//
//    std::vector<std::thread> threads;
//    for(std::size_t i = 0; i < 10; i++) {
//        threads.emplace_back(&testUpdateBlock, std::ref(manager), i);
//    }
//    for(auto& thread: threads) {
//        thread.join();
//    }
//}
