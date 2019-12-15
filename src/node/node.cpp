#include "node.hpp"

#include <functional>

Node::Node(const base::PropertyTree& config) : _config{config}, _core{_config}
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


void Node::onNewTransactionReceived(const bc::Transaction& tx)
{
    if(_block_to_mine.getTransactions().isEmpty()) {
        _block_to_mine.setPrevBlockHash(base::Sha256::compute(base::toBytes(_core.getTopBlock())).getBytes());
    }
    _block_to_mine.addTransaction(tx);
    base::Bytes complexity(32);
    complexity[2] = 0x4f;
    _miner->findNonce(_block_to_mine, complexity);
}


void Node::onNewBlock(const bc::Block& block)
{
    auto tset = _block_to_mine.getTransactions();
    tset.remove(block.getTransactions());
    if(!tset.isEmpty() && tset.size() != _block_to_mine.getTransactions().size()) {
        _block_to_mine.setTransactions(std::move(tset));
        _block_to_mine.setPrevBlockHash(base::Sha256::compute(base::toBytes(block)).getBytes());
        base::Bytes complexity(32);
        complexity[2] = 0x4f;
        _miner->findNonce(_block_to_mine, complexity);
    }
}
