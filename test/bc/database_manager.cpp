#include <boost/test/unit_test.hpp>

#include "bc/database_manager.hpp"

#include <fstream>

namespace
{
auto hash1 = base::Sha256::compute(base::Bytes("first"));
auto hash2 = base::Sha256::compute(base::Bytes("second"));
auto hash3 = base::Sha256::compute(base::Bytes("third"));

std::vector<bc::Block> getInitBlocks()
{
    std::vector<bc::Block> blocks;

    blocks.push_back(bc::Block(hash1, bc::TransactionsSet()));

    bc::TransactionsSet set1;
    set1.add(bc::Transaction(bc::Address("from1"), bc::Address("to1"), bc::Balance(100), base::Time::now()));
    blocks.push_back(bc::Block(hash2, set1));

    bc::TransactionsSet set2;
    set2.add(bc::Transaction(bc::Address("from2"), bc::Address("to2"), bc::Balance(133), base::Time::now()));
    set2.add(bc::Transaction(bc::Address("from3"), bc::Address("to3"), bc::Balance(85), base::Time()));
    blocks.push_back(bc::Block(hash3, set2));
    return blocks;
}

std::vector<bc::Transaction> getTransactionFromBlock(const bc::Block& block)
{
    std::vector<bc::Transaction> transactions;
    for(const auto& transaction: block.getTransactions()) {
        transactions.push_back(transaction);
    }
    return transactions;
}
} // namespace

BOOST_AUTO_TEST_CASE(database_manager_usage1)
{
    {
        std::ofstream output("config.json");
        output << R"({
    "database": {
        "path": "likelib/local_test_base",
        "clean": false
    }
})";
    }
    base::PropertyTree config = base::readConfig("config.json");

    bc::DatabaseManager manager(config);

    auto blocks = getInitBlocks();
    for(const auto& block: blocks) {
        manager.addBlock(block.getPrevBlockHash(), block);
    }

    BOOST_CHECK(manager.findBlock(hash1) != std::nullopt);
    BOOST_CHECK(manager.findBlock(hash2) != std::nullopt);
    BOOST_CHECK(manager.findBlock(hash3) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hash3);

    std::filesystem::remove_all("likelib/local_test_base");
    std::filesystem::remove("config.json");
}


BOOST_AUTO_TEST_CASE(database_manager_usage2)
{
    {
        std::ofstream output("config.json");
        output << R"({
    "database": {
        "path": "likelib/local_test_base",
        "clean": false
    }
})";
    }
    base::PropertyTree config = base::readConfig("config.json");

    bc::DatabaseManager manager(config);

    auto blocks = getInitBlocks();
    manager.addBlock(blocks[0].getPrevBlockHash(), blocks[0]);
    BOOST_CHECK(manager.findBlock(hash1) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hash1);

    manager.addBlock(blocks[1].getPrevBlockHash(), blocks[1]);
    BOOST_CHECK(manager.findBlock(hash2) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hash2);

    manager.addBlock(blocks[2].getPrevBlockHash(), blocks[2]);
    BOOST_CHECK(manager.findBlock(hash3) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hash3);

    std::filesystem::remove_all("likelib/local_test_base");
    std::filesystem::remove("config.json");
}


BOOST_AUTO_TEST_CASE(database_manager_check_transaction_in_blocks)
{
    {
        std::ofstream output("config.json");
        output << R"({
    "database": {
        "path": "likelib/local_test_base",
        "clean": false
    }
})";
    }
    base::PropertyTree config = base::readConfig("config.json");

    bc::DatabaseManager manager(config);

    auto blocks = getInitBlocks();
    for(const auto& block: blocks) {
        manager.addBlock(block.getPrevBlockHash(), block);
    }

    auto trans1 = getTransactionFromBlock(blocks[0]);
    auto trans11 = getTransactionFromBlock(manager.findBlock(hash1).value()); // TODO:make it with for
    bool res = true;
    for(std::size_t i = 0; i < trans1.size(); i++) {
        res = res && (trans1 == trans11);
    }
    BOOST_CHECK(res);

    res = true;
    auto trans2 = getTransactionFromBlock(blocks[1]);
    auto trans22 = getTransactionFromBlock(manager.findBlock(hash2).value());
    for(std::size_t i = 0; i < trans1.size(); i++) {
        res = res && (trans2 == trans22);
    }
    BOOST_CHECK(res);

    res = true;
    auto trans3 = getTransactionFromBlock(blocks[2]);
    auto trans33 = getTransactionFromBlock(manager.findBlock(hash3).value());
    for(std::size_t i = 0; i < trans1.size(); i++) {
        res = res && (trans3 == trans33);
    }
    BOOST_CHECK(res);

    std::filesystem::remove_all("likelib/local_test_base");
    std::filesystem::remove("config.json");
}


BOOST_AUTO_TEST_CASE(database_manager_with_not_empty_database)
{
    {
        std::ofstream output("config.json");
        output << R"({
    "database": {
        "path": "likelib/local_test_base",
        "clean": false
    }
})";
    }
    base::PropertyTree config = base::readConfig("config.json");

    {
        bc::DatabaseManager manager(config);

        auto blocks = getInitBlocks();
        for(const auto& block: blocks) {
            manager.addBlock(block.getPrevBlockHash(), block);
        }
    }

    bc::DatabaseManager manager(config);
    BOOST_CHECK(manager.findBlock(hash1) != std::nullopt);
    BOOST_CHECK(manager.findBlock(hash2) != std::nullopt);
    BOOST_CHECK(manager.findBlock(hash3) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hash3);

    std::filesystem::remove_all("likelib/local_test_base");
    std::filesystem::remove("config.json");
}


BOOST_AUTO_TEST_CASE(database_manager_with_not_empty_database_and_clean)
{
    {
        std::ofstream output("config.json");
        output << R"({
    "database": {
        "path": "likelib/local_test_base",
        "clean": true
    }
})";
    }
    base::PropertyTree config = base::readConfig("config.json");

    {
        bc::DatabaseManager manager(config);

        auto blocks = getInitBlocks();
        for(const auto& block: blocks) {
            manager.addBlock(block.getPrevBlockHash(), block);
        }
    }

    bc::DatabaseManager manager(config);
    BOOST_CHECK(manager.findBlock(hash1) == std::nullopt);
    BOOST_CHECK(manager.findBlock(hash2) == std::nullopt);
    BOOST_CHECK(manager.findBlock(hash3) == std::nullopt);

    std::filesystem::remove_all("likelib/local_test_base");
    std::filesystem::remove("config.json");
}