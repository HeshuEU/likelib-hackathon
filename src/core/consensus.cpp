#include "consensus.hpp"

#include "base/config.hpp"

namespace lk
{

base::FixedBytes<Complexity::LENGTH> Complexity::calcComparer(const base::Uint256& densed)
{
    base::FixedBytes<LENGTH> ret;
    std::size_t index = LENGTH;
    for(auto d = densed; d > 0; d /= 256) {
        ret[--index] = (d % 256).toMultiNumber().convert_to<base::Byte>();
    }
    return ret;
}


Complexity::Complexity(base::Uint256 densed)
    : _densed{ std::move(densed) },
      _comparer{ calcComparer(_densed) }  {
}


const base::Uint256& Complexity::getDensed() const noexcept
{
    return _densed;
}


const base::FixedBytes<Complexity::LENGTH>& Complexity::getComparer() const
{
    return _comparer;
}


Consensus::Consensus()
    : _complexity{~base::Uint256{0}}
{}


const Complexity& Consensus::getComplexity() const
{
    return _complexity;
}


bool Consensus::checkBlock(const Block &block) const
{
    return base::Sha256::compute(base::toBytes(block)).getBytes() <= _complexity.getComparer();
}


void Consensus::applyBlock(const Block& block)
{
    _last_blocks.push(&block);
    if(_last_blocks.size() < base::config::BC_DIFFICULTY_RECALCULATION_RATE) {
        // means we do not have enough block to recalculate anything
        return;
    }

    if(_last_blocks.size() > base::config::BC_DIFFICULTY_RECALCULATION_RATE) {
        _last_blocks.pop();
    }

    if(block.getDepth() % base::config::BC_DIFFICULTY_RECALCULATION_RATE != 0) {
        return;
    }

    const Block& p = *_last_blocks.front();

    auto elapsed = (block.getTimestamp() - p.getTimestamp()).getSeconds();
    static const int TARGET = base::config::BC_DIFFICULTY_RECALCULATION_RATE * 60 / base::config::BC_TARGET_BLOCKS_PER_MINUTE;

    _complexity = Complexity{ _complexity.getDensed() * TARGET / elapsed };
}


}