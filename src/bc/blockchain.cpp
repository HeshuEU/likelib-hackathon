#include "blockchain.hpp"

#include <base/assert.hpp>


namespace bc
{

void Blockchain::blockReceived(Block&& block)
{
    ASSERT(!_blocks.empty());
    if(block.checkValidness() && _blocks.back().getHash() == block.getPrevBlockHash()) {
        _blocks.push_back(std::move(block));
    }
}


void Blockchain::transactionReceived(Transaction&& transaction)
{}

} // namespace bc