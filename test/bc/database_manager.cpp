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
    blocks.push_back(bc::Block(hash3, set2));
    return blocks;
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
    BOOST_CHECK(config.hasKey("database.path"));

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