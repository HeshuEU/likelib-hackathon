#include "node.hpp"

#include <functional>

Node::Node(const base::PropertyTree& config) : _config{config}, _key_vault(_config), _core{_config, _key_vault}
{
    auto service = std::make_shared<node::GeneralServerService>(_core);
    _rpc = std::make_unique<rpc::RpcServer>(_config.get<std::string>("rpc.address"), service);

    auto miner_callback = std::bind(&Node::onBlockMine, this, std::placeholders::_1);
    _miner = std::make_unique<Miner>(_config, miner_callback);

    _core.subscribeToNewPendingTransaction(std::bind(&Node::onNewTransactionReceived, this, std::placeholders::_1));
    _core.subscribeToBlockAddition(std::bind(&Node::onNewBlock, this, std::placeholders::_1));
}


void Node::run()
{
    _core.run(); // run before all others

    try {
        _rpc->run();
        LOG_INFO << "RPC server started: " << _config.get<std::string>("rpc.address");
    }
    catch(const std::exception& e) {
        LOG_WARNING << "Cannot start RPC server: " << e.what();
    }
    catch(...) {
        LOG_WARNING << "Cannot start RPC server: unknown error";
    }
}


void Node::onBlockMine(bc::Block&& block)
{
    _core.tryAddBlock(block);
}


base::FixedBytes<impl::CommonData::COMPLEXITY_SIZE> Node::getMiningComplexity()
{
    base::FixedBytes<impl::CommonData::COMPLEXITY_SIZE> complexity;
    complexity[2] = 0xff;
    return complexity;
}


void Node::onNewTransactionReceived(const bc::Transaction&)
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
