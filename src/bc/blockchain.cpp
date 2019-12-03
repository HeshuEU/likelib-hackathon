#include "blockchain.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "net/network.hpp"


namespace bc
{

Blockchain::Blockchain(const base::PropertyTree& config)
    : _config(config), _miner(config, std::bind(&Blockchain::onMinerFinished, this, std::placeholders::_1)),
      _network_handler(*this)
{
    setupGenesis();
    _network = std::make_unique<net::Network>(config, _network_handler);
}


void Blockchain::processReceivedBlock(Block&& block)
{
    LOG_DEBUG << "Block received. Block hash = " << base::Sha256::compute(base::toBytes(block)).getBytes().toHex();
    {
        std::lock_guard lk(_blocks_mutex);
        ASSERT(!_blocks.empty());
    }

    if(checkBlock(block)) {
        _miner.dropJob();
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
    {
        std::lock_guard lk(_blocks_mutex);
        _blocks.push_back(block);
    }
    std::lock_guard lk(_balance_manager_mutex);
    for(const auto& tx: block.getTransactions()) {
        if(_balance_manager.checkTransaction(tx)) {
            _balance_manager.update(tx);
        }
    }
}


void Blockchain::processReceivedTransaction(Transaction&& transaction)
{
    LOG_DEBUG << "Received transaction. From: " << transaction.getFrom().toString() << " To: "
              << transaction.getTo().toString() << " Amount:" << transaction.getAmount();

    if(!checkTransaction(transaction)) {
        LOG_INFO << "Received an invalid transaction";
        return;
    }
    else {
        std::lock_guard lk(_pending_block_mutex);
        if(!_pending_block.getTransactions().find(transaction)) {
            _pending_block.addTransaction(transaction);
            {
                std::lock_guard lk1(_blocks_mutex);
                _pending_block.setPrevBlockHash(base::Sha256::compute(base::toBytes(_blocks.back())).getBytes());
            }
            _network->broadcastTransaction(transaction);
            _miner.dropJob();
            _miner.findNonce(_pending_block);
        }
    }
}


bc::Balance Blockchain::getBalance(const bc::Address& address) const
{
    std::lock_guard lk(_balance_manager_mutex);
    return _balance_manager.getBalance(address);
}


void Blockchain::setupGenesis()
{
    Block genesis;
    genesis.setNonce(0);
    genesis.setPrevBlockHash(base::Bytes(32));
    {
        std::lock_guard lk(_blocks_mutex);
        _blocks.push_back(std::move(genesis));
    }
}


base::Bytes Blockchain::getMiningComplexity() const
{
    base::Bytes ret(32);
    ret[2] = 0x6F;
    return ret;
}


void Blockchain::onMinerFinished(Block block)
{
    addBlock(block);
    _network->broadcastBlock(block);
}


bool Blockchain::checkTransaction(const Transaction& tx) const
{
    return true; // TODO: implement transaction verification (having same transaction and etc.)
}


bool Blockchain::checkBlock(const Block& block) const
{
    const base::Sha256 block_hash = base::Sha256::compute(base::toBytes(block));
    std::lock_guard lk(_blocks_mutex);
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


//=======================================

Blockchain::NetworkHandler::NetworkHandler(Blockchain& bc) : _bc{bc}
{}


void Blockchain::NetworkHandler::onBlockReceived(Block&& block)
{
    _bc.processReceivedBlock(std::move(block));
}


void Blockchain::NetworkHandler::onTransactionReceived(Transaction&& transaction)
{
    _bc.processReceivedTransaction(std::move(transaction));
}

//=======================================

} // namespace bc