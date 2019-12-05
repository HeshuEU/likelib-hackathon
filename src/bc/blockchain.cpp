#include "blockchain.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "net/network.hpp"

namespace
{

const bc::Block& generateGenesis()
{
    static bc::Block genesis;
    genesis.setNonce(0);
    genesis.setPrevBlockHash(base::Bytes(32));

    static const bc::Address BASE_ADDRESS{std::string(32, '0')};
    static const bc::Balance BASE_MONEY_AMOUNT = 0xffffffff;

    genesis.addTransaction({BASE_ADDRESS, BASE_ADDRESS, BASE_MONEY_AMOUNT, base::Time()});
    return genesis;
}

} // namespace

namespace bc
{

Blockchain::Blockchain(const base::PropertyTree& config)
    : _config(config), _database(config),
      _miner(config, std::bind(&Blockchain::onMinerFinished, this, std::placeholders::_1)), _network_handler(*this)
{
    setupGenesis();
    setupBalanceManager();
    _network = std::make_unique<net::Network>(config, _network_handler);
}


void Blockchain::processReceivedBlock(Block&& block)
{
    LOG_DEBUG << "Block received. Block hash = " << base::Sha256::compute(base::toBytes(block)).getBytes().toHex();
    ASSERT(_database.getLastBlockHash() != base::Bytes(32));

    if(checkBlock(block)) {
        _miner.dropJob();
        addBlock(block);
        auto pending_txs = _pending_block.getTransactions();
        pending_txs.remove(block.getTransactions());
        _pending_block.setTransactions(std::move(pending_txs));
        if(!_pending_block.getTransactions().isEmpty()) {
            _miner.findNonce(_pending_block, getMiningComplexity());
        }
    }
}


void Blockchain::addBlock(const Block& block)
{
    auto block_hash = base::Sha256::compute(base::toBytes(block));
    LOG_DEBUG << "Adding block. Block hash = " << block_hash.getBytes().toHex();
    _database.addBlock(block_hash, block);
    std::lock_guard lk(_pending_block_mutex);
    auto pending_transactions = _pending_block.getTransactions();
    for(const auto& tx: block.getTransactions()) {
        if(pending_transactions.find(tx)) {
            pending_transactions.remove(tx);
        }
        if(_balance_manager.checkTransaction(tx)) {
            _balance_manager.update(tx);
        }
    }
    _pending_block.setTransactions(std::move(pending_transactions));
}


bool Blockchain::processReceivedTransaction(Transaction&& transaction)
{
    LOG_DEBUG << "Received transaction. From: " << transaction.getFrom().toString()
              << " To: " << transaction.getTo().toString() << " Amount:" << transaction.getAmount()
              << " Seconds from epoch:" << transaction.getTimestamp().secondsInEpoch();

    if(!checkTransaction(transaction)) {
        LOG_INFO << "Received an invalid transaction";
        return false;
    }
    else {
        std::lock_guard lk(_pending_block_mutex);
        if(!_pending_block.getTransactions().find(transaction)) {
            _pending_block.addTransaction(transaction);
            _pending_block.setPrevBlockHash(_database.getLastBlockHash().getBytes());
            _network->broadcastTransaction(transaction);
            _miner.dropJob();
            _miner.findNonce(_pending_block, getMiningComplexity());
            return true;
        }
        return false;
    }
}


bc::Balance Blockchain::getBalance(const bc::Address& address) const
{
    return _balance_manager.getBalance(address);
}


void Blockchain::setupGenesis()
{
    if(_database.getLastBlockHash() == base::Bytes(32)) {
        auto genesis = generateGenesis();
        auto genesis_hash = base::Sha256::compute(base::toBytes(genesis));
        _database.addBlock(genesis_hash, genesis);
        _balance_manager.updateFromGenesis(genesis);
    }
    else {
        auto genesis_hash = base::Sha256::compute(base::toBytes(generateGenesis()));
        auto all_blocks_hashes = _database.createAllBlockHashesList();
        if(genesis_hash != all_blocks_hashes.front()) {
            RAISE_ERROR(base::LogicError, "genesis block does not match genesis block from database");
        }
    }
}


void Blockchain::setupBalanceManager()
{
    auto all_blocks_hashes = _database.createAllBlockHashesList();

    _balance_manager.updateFromGenesis(_database.getBlock(all_blocks_hashes.front()));

    if(all_blocks_hashes.size() > 1) {
        for(std::size_t i = 0; i < all_blocks_hashes.size(); i++) {
            _balance_manager.update(_database.getBlock(all_blocks_hashes[i]));
        }
    }
}


base::Bytes Blockchain::getMiningComplexity() const
{
    // to be changed later to calculate complexity dynamically
    base::Bytes ret(32);
    ret[2] = 0x6F;
    return ret;
}


void Blockchain::onMinerFinished(Block&& block)
{
    addBlock(block);
    _network->broadcastBlock(block);
}


bool Blockchain::checkTransaction(const Transaction& tx) const
{
    if(!_balance_manager.checkTransaction(tx)) {
        return false;
    }
    {
        // TODO: optimize it of course
        auto all_blocks_hashes = _database.createAllBlockHashesList();
        for(const auto& block_hash: all_blocks_hashes) {
            auto block = _database.getBlock(block_hash);
            for(const auto& transaction: block.getTransactions()) {
                if(tx == transaction) {
                    return false;
                }
            }
        }
    }
    return true;
}


bool Blockchain::checkBlock(const Block& block) const
{
    const base::Sha256 block_hash = base::Sha256::compute(base::toBytes(block));

    if(_database.getLastBlockHash().getBytes() != block.getPrevBlockHash()) {
        // received block doesn't match our last
        return false;
    }

    for(const auto& tx: block.getTransactions()) {
        if(!checkTransaction(tx)) {
            return false;
        }
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