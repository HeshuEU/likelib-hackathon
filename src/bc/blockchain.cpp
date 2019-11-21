#include "blockchain.hpp"

#include "net/network.hpp"

#include <base/assert.hpp>


namespace bc
{


Blockchain::Blockchain()
{
    Block genesis;
    genesis.setNonce(0);
    genesis.setPrevBlockHash(base::Bytes(32));
    _blocks.push_back(genesis);
}


void Blockchain::blockReceived(Block&& block)
{
    ASSERT(!_blocks.empty());
    if(block.checkValidness() &&
        base::Sha256::calcSha256(base::toBytes(_blocks.back())).getBytes() == block.getPrevBlockHash()) {
        addBlock(block);
    }
}


void Blockchain::addBlock(const Block& block)
{
    _blocks.push_back(std::move(block));
}


void Blockchain::transactionReceived(Transaction&& transaction)
{
    _pending_block.setTransactions({std::move(transaction)});
    _miner.findNonce(_pending_block, [this](std::optional<Block> block) {
        if(block) {
            _network->broadcastBlock(block.value());
        }
    });
}


void Blockchain::setNetwork(net::Network* network)
{
    _network = network;
}

} // namespace bc