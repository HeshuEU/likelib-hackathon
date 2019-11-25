#include "blockchain.hpp"

#include "base/assert.hpp"
#include "net/network.hpp"


namespace bc
{


Blockchain::Blockchain()
{
    Block genesis;
    genesis.setNonce(0);
    genesis.setPrevBlockHash(base::Bytes(32));
    genesis.addTransaction({"00000000000000000000000000000000", "11111111111111111111111111111111", 150});
    _blocks.push_back(genesis);

    _miner.setCallback([this](std::optional<Block> block) {
        if(block) {
            addBlock(*block);
            _network->broadcastBlock(*block);
        }
    });
}


void Blockchain::blockReceived(Block&& block)
{
    LOG_DEBUG << "Block received. Block hash = " << base::Sha256::compute(base::toBytes(block)).getBytes().toHex();
    ASSERT(!_blocks.empty());
    if(block.checkValidness() &&
        base::Sha256::compute(base::toBytes(_blocks.back())).getBytes() == block.getPrevBlockHash()) {
        _miner.stop();
        addBlock(block);
        std::vector<Transaction> new_txs;
        for(const auto& tx : _pending_block.getTransactions()) {
            if(std::find_if(block.getTransactions().begin(), block.getTransactions().end(), [&tx](const Transaction& tx1) {
                return base::Sha256::compute(base::toBytes(tx)) == base::Sha256::compute(base::toBytes(tx1));
            }) == block.getTransactions().end()) {
                new_txs.push_back(tx);
            }
        }
        _pending_block.setTransactions(std::move(new_txs));
        _miner.findNonce(_pending_block);
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


void Blockchain::transactionReceived(Transaction&& transaction)
{
    LOG_DEBUG << "Received transaction. From = " << transaction.getFrom().toString() << ' ' << transaction.getTo().toString() << ' ' << transaction.getAmount();
    _pending_block.setPrevBlockHash(base::Sha256::compute(base::toBytes(_blocks.back())).getBytes());
    _pending_block.setTransactions({std::move(transaction)});
    _miner.stop();
    _miner.findNonce(_pending_block);
}


bc::Balance Blockchain::getBalance(const bc::Address& address) const
{
    return _balance_manager.getBalance(address);
}


void Blockchain::setNetwork(net::Network* network)
{
    _network = network;
}

} // namespace bc