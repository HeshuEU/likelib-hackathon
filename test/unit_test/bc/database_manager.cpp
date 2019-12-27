#include <boost/test/unit_test.hpp>

#include "bc/database_manager.hpp"

#include <fstream>
#include <vector>

namespace
{

std::vector<base::Sha256> hashes{base::Sha256::compute(base::Bytes("first")),
    base::Sha256::compute(base::Bytes("second")), base::Sha256::compute(base::Bytes("third"))};

std::vector<bc::Block> getInitBlocks()
{
    std::vector<bc::Block> blocks;

    blocks.push_back(bc::Block(hashes[0], bc::TransactionsSet()));

    bc::TransactionsSet set1;
    set1.add(bc::Transaction(bc::Address("from1"), bc::Address("to1"), bc::Balance(100), base::Time::now()));
    blocks.push_back(bc::Block(hashes[1], set1));

    bc::TransactionsSet set2;
    set2.add(bc::Transaction(bc::Address("from2"), bc::Address("to2"), bc::Balance(133), base::Time::now()));
    set2.add(bc::Transaction(bc::Address("from3"), bc::Address("to3"), bc::Balance(85), base::Time()));
    blocks.push_back(bc::Block(hashes[2], set2));
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

    BOOST_CHECK(manager.findBlock(hashes[0]) != std::nullopt);
    BOOST_CHECK(manager.findBlock(hashes[1]) != std::nullopt);
    BOOST_CHECK(manager.findBlock(hashes[2]) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hashes[2]);

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
    BOOST_CHECK(manager.findBlock(hashes[0]) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hashes[0]);

    manager.addBlock(blocks[1].getPrevBlockHash(), blocks[1]);
    BOOST_CHECK(manager.findBlock(hashes[1]) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hashes[1]);

    manager.addBlock(blocks[2].getPrevBlockHash(), blocks[2]);
    BOOST_CHECK(manager.findBlock(hashes[2]) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hashes[2]);

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

    bool res = true;
    for(std::size_t i = 0; i < blocks.size(); i++) {
        auto trans1 = getTransactionFromBlock(blocks[i]);
        auto trans1_set = blocks[i].getTransactions();
        auto trans2 = getTransactionFromBlock(manager.findBlock(hashes[i]).value());
        res = res && (trans1.size() == trans2.size());
        for(std::size_t j = 0; j < trans2.size(); j++) {
            res = res && (trans1_set.find(trans2[j]));
        }
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
    BOOST_CHECK(manager.findBlock(hashes[0]) != std::nullopt);
    BOOST_CHECK(manager.findBlock(hashes[1]) != std::nullopt);
    BOOST_CHECK(manager.findBlock(hashes[2]) != std::nullopt);
    BOOST_CHECK(manager.getLastBlockHash() == hashes[2]);

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
    BOOST_CHECK(manager.findBlock(hashes[0]) == std::nullopt);
    BOOST_CHECK(manager.findBlock(hashes[1]) == std::nullopt);
    BOOST_CHECK(manager.findBlock(hashes[2]) == std::nullopt);

    std::filesystem::remove_all("likelib/local_test_base");
    std::filesystem::remove("config.json");
}