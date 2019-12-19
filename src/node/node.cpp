#include "node.hpp"

#include <functional>

Node::Node(const base::PropertyTree& config) : _config{config}, _core{_config}, _key_vault(_config)
{
    auto service = std::make_shared<node::GeneralServerService>(_core);
    _rpc = std::make_unique<rpc::RpcServer>(_config.get<std::string>("rpc.address"), service);

    auto miner_callback = std::bind(&Node::onBlockMine, this, std::placeholders::_1);
    _miner = std::make_unique<Miner>(_config, miner_callback);

    _core.signal_new_transaction.connect(std::bind(&Node::onNewTransactionReceived, this, std::placeholders::_1));
    _core.signal_new_block.connect(std::bind(&Node::onNewBlock, this, std::placeholders::_1));
}


void Node::run()
{
    _core.run(); // run before all others

    _rpc->run();
    LOG_INFO << "RPC server started: " << _config.get<std::string>("rpc.address");
}


void Node::onBlockMine(bc::Block&& block)
{
    _core.tryAddBlock(block);
}


base::Bytes Node::getMiningComplexity()
{
    base::Bytes complexity(32);
    complexity[2] = 0x8f;
    return complexity;
}


void Node::onNewTransactionReceived(const bc::Transaction& tx)
{
    bc::Block block = _core.getBlockTemplate();
    if(!block.getTransactions().isEmpty()) {
        _miner->findNonce(_core.getBlockTemplate(), getMiningComplexity());
    }
    else {
        _miner->dropJob();
    }
}


void Node::onNewBlock(const bc::Block&)
{
    bc::Block block = _core.getBlockTemplate();
    if(!block.getTransactions().isEmpty()) {
        _miner->findNonce(_core.getBlockTemplate(), getMiningComplexity());
    }
    else {
        _miner->dropJob();
    }
}
