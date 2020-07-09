#include "node.hpp"

#include <functional>

Node::Node(const base::PropertyTree& config)
  : _config{ config }
  , _key_vault(_config)
  , _core{ _config, _key_vault }
  , _rpc{ _config, _core }
{
    _miner = std::make_unique<Miner>(_config, std::bind(&Node::onBlockMine, this, std::placeholders::_1));

    _core.subscribeToNewPendingTransaction(std::bind(&Node::onNewTransactionReceived, this, std::placeholders::_1));
    _core.subscribeToBlockAddition(std::bind(&Node::onNewBlock, this, std::placeholders::_1));
}


void Node::run()
{
    _core.run(); // run before all others

    try {
        _rpc.run();
    }
    catch (const std::exception& e) {
        LOG_WARNING << "Cannot startSession RPC server: " << e.what();
    }
    catch (...) {
        LOG_WARNING << "Cannot startSession RPC server: unknown error";
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
