#include "blockchain.hpp"

#include "base/assert.hpp"
#include "net/network.hpp"


namespace bc
{

Blockchain::Blockchain(const base::PropertyTree& config)
    : _config(config)
{
    setupGenesis();
    _miner.setCallback(std::bind(&Blockchain::onMinerFinished, this, std::placeholders::_1));
    _network = std::make_unique<net::Network>(net::Endpoint{config.get<std::string>("listen_address")},
                                              config.get<unsigned short>("public_server_port"));
}


void Blockchain::processReceivedBlock(Block&& block)
{
    LOG_DEBUG << "Block received. Block hash = " << base::Sha256::compute(base::toBytes(block)).getBytes().toHex();
    ASSERT(!_blocks.empty());
    if(checkBlock(block)) {
        _miner.stop();
        addBlock(block);
        auto pending_txs = _pending_block.getTransactions();
        pending_txs.remove(block.getTransactions());
        _pending_block.setTransactions(std::move(pending_txs));
        if(!_pending_block.getTransactions().isEmpty()) {
            _miner.findNonce(_pending_block);
        }
    }
}


void Blockchain::addBlock(const Block& block)
{
    LOG_DEBUG << "Adding block. Block hash = " << base::Sha256::compute(base::toBytes(block)).getBytes().toHex();
    _blocks.push_back(std::move(block));
    for(const auto& tx: block.getTransactions()) {
        if(_balance_manager.checkTransaction(tx)) {
            _balance_manager.update(tx);
        }
    }
}


void Blockchain::processReceivedTransaction(Transaction&& transaction)
{
    LOG_DEBUG << "Received transaction. From = " << transaction.getFrom().toString() << ' '
              << transaction.getTo().toString() << ' ' << transaction.getAmount();

    if(!checkTransaction(transaction)) {
        LOG_INFO << "Received an invalid transaction";
        return;
    }
    else {
        if (!_pending_block.getTransactions().find(transaction)) {
            _pending_block.addTransaction({std::move(transaction)});
            _pending_block.setPrevBlockHash(base::Sha256::compute(base::toBytes(_blocks.back())).getBytes());
            _network->broadcastTransaction(transaction);
            _miner.stop();
            _miner.findNonce(_pending_block);
        }
    }
}


bc::Balance Blockchain::getBalance(const bc::Address& address) const
{
    return _balance_manager.getBalance(address);
}


void Blockchain::setupGenesis()
{
    Block genesis;
    genesis.setNonce(0);
    genesis.setPrevBlockHash(base::Bytes(32));
    _blocks.push_back(std::move(genesis));
}


void Blockchain::onMinerFinished(const std::optional<Block>& block)
{
    if(block) {
        addBlock(*block);
        _network->broadcastBlock(*block);
    }
}


bool Blockchain::checkTransaction(const Transaction& tx) const
{
    return true; // TODO: implement transaction verification (having same transaction and etc.)
}


bool Blockchain::checkBlock(const Block& block) const
{
    const base::Sha256 block_hash = base::Sha256::compute(base::toBytes(block));
    if(base::Sha256::compute(base::toBytes(_blocks.back())) != block.getPrevBlockHash()) {
        // received block doesn't match our last
        return false;
    }
    return true;
}


void Blockchain::run()
{
    // run network processing loop
    _network->run();

    // connect to all nodes, given in configuration file
    for(const auto& node_ip_string: _config.getVector<std::string>("nodes")) {
        _network->connect(net::Endpoint{node_ip_string});
    }
}

} // namespace bc