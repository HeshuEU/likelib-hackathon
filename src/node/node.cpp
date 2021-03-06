#include "node.hpp"

#include <functional>
#include <string>

Node::Node(base::json::Value config)
  : _config{ std::move(config) }
  , _core{ std::move(_config["core"]) }
  , _public_service{ std::move(_config["websocket"]), _core }
{
    if (!_config.has_object_field("miner")) {
        RAISE_ERROR(base::InvalidArgument, "config file is't contain miner node");
    }
    _miner = std::make_unique<Miner>(std::move(_config["miner"]),
                                     std::bind(&Node::onBlockMine, this, std::placeholders::_1));

    _core.subscribeToNewPendingTransaction(std::bind(&Node::onNewTransactionReceived, this, std::placeholders::_1));
    _core.subscribeToBlockAddition(std::bind(&Node::onNewBlock, this, std::placeholders::_1));
}


void Node::run()
{
    _core.run(); // run before all others

    try {
        _public_service.run();
    }
    catch (const std::exception& e) {
        LOG_WARNING << "Cannot startSession public server: " << e.what();
    }
    catch (...) {
        LOG_WARNING << "Cannot startSession public server: unknown error";
    }
}


void Node::onBlockMine(lk::ImmutableBlock&& block)
{
    LOG_DEBUG << "Block " << base::Sha256::compute(base::toBytes(block)) << " mined";
    [[maybe_unused]] auto r = _core.tryAddMinedBlock(block);
    if (r != lk::Blockchain::AdditionResult::ADDED) {
        LOG_DEBUG << "Block " << base::Sha256::compute(base::toBytes(block)) << " addition resulted in error code "
                  << static_cast<int>(r);
    }
}


void Node::onNewTransactionReceived(const lk::Transaction&)
{
    auto [block, complexity] = _core.getMiningData();
    if (!block.getTransactions().isEmpty()) {
        _miner->findNonce(block, complexity);
    }
    else {
        _miner->dropJob();
    }
}


void Node::onNewBlock(const lk::ImmutableBlock&)
{
    auto [block, complexity] = _core.getMiningData();
    if (!block.getTransactions().isEmpty()) {
        _miner->findNonce(block, complexity);
    }
    else {
        _miner->dropJob();
    }
}
