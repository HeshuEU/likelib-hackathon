#include "blockchain.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "net/host.hpp"

#include <optional>

namespace bc
{

Blockchain::Blockchain(const base::PropertyTree& config)
    : _config(config), _miner(config, std::bind(&Blockchain::onMinerFinished, this, std::placeholders::_1)),
      _host{config}
{
    setupGenesis();
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
            _miner.findNonce(_pending_block, getMiningComplexity());
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
    for(const auto& tx: block.getTransactions()) {
        if(_balance_manager.checkTransaction(tx)) {
            _balance_manager.update(tx);
        }
    }
}


std::optional<Block> Blockchain::findBlock(const base::Sha256& block_hash) const
{
    std::shared_lock lk(_blocks_mutex);
    for(const auto& block: _blocks) {
        if(base::Sha256::compute(base::toBytes(block)) == block_hash) {
            return block;
        }
    }
    return std::nullopt;
}


void Blockchain::processReceivedTransaction(Transaction&& transaction)
{
    LOG_DEBUG << "Received transaction. From: " << transaction.getFrom().toString()
              << " To: " << transaction.getTo().toString() << " Amount:" << transaction.getAmount();

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
            broadcastTransaction(transaction);
            _miner.dropJob();
            _miner.findNonce(_pending_block, getMiningComplexity());
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

    static const bc::Address BASE_ADDRESS{std::string(32, '0')};
    static const bc::Balance BASE_MONEY_AMOUNT = 0xffffffff;
    genesis.addTransaction({BASE_ADDRESS, BASE_ADDRESS, BASE_MONEY_AMOUNT, base::Time()});

    _balance_manager.updateFromGenesis(genesis);
    {
        std::lock_guard lk(_blocks_mutex);
        _blocks.push_back(std::move(genesis));
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
    broadcastBlock(block);
}


void Blockchain::broadcastBlock(const bc::Block& block)
{
    _host.broadcastDataPacket(base::toBytes(block));
}


void Blockchain::broadcastTransaction(const bc::Transaction& tx)
{
    _host.broadcastDataPacket(base::toBytes(tx));
}


bool Blockchain::checkTransaction(const Transaction& tx) const
{
    if(!_balance_manager.checkTransaction(tx)) {
        return false;
    }
    {
        // TODO: optimize it of course
        std::lock_guard lk(_blocks_mutex);
        for(const auto& block: _blocks) {
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
    std::lock_guard lk(_blocks_mutex);
    if(base::Sha256::compute(base::toBytes(_blocks.back())) != block.getPrevBlockHash()) {
        // received block doesn't match our last
        return false;
    }

    for(const auto& tx: block.getTransactions()) {
        if(!_balance_manager.checkTransaction(tx)) {
            return false;
        }
    }

    return true;
}


void Blockchain::run()
{
    // run network processing loop
    _host.run();

    // connect to all nodes, given in configuration file
    for(const auto& node_ip_string: _config.getVector<std::string>("nodes")) {
        _host.connect(net::Endpoint{node_ip_string});
    }
}

} // namespace bc