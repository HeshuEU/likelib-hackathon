#include "consensus.hpp"

#include "base/config.hpp"

namespace lk
{

Complexity::Comparer Complexity::calcComparer(const Complexity::Densed& densed)
{
    Complexity::Comparer ret;
    std::size_t index = LENGTH;
    for (auto d = densed; d > 0; d /= 256) {
        ret[--index] = (d % 256).toMultiprecisionNumber().convert_to<base::Byte>();
    }
    return ret;
}


Complexity::Complexity(Complexity::Densed densed)
  : _densed{ std::move(densed) }
  , _comparer{ calcComparer(_densed) }
{}


const Complexity::Densed& Complexity::getDensed() const noexcept
{
    return _densed;
}


const Complexity::Comparer& Complexity::getComparer() const noexcept
{
    return _comparer;
}


Consensus::Consensus()
  : _complexity{ ~base::Uint256{ 0 } }
{}


const Complexity& Consensus::getComplexity() const
{
    return _complexity;
}


bool Consensus::checkBlock(const Block& block) const
{
    return base::Sha256::compute(base::toBytes(block)).getBytes() <= _complexity.getComparer();
}


void Consensus::applyBlock(const Block& block)
{
    _last_blocks.push(&block);
    if (_last_blocks.size() < base::config::BC_DIFFICULTY_RECALCULATION_RATE) {
        // means we do not have enough block to recalculate anything
        return;
    }

    if (_last_blocks.size() > base::config::BC_DIFFICULTY_RECALCULATION_RATE) {
        _last_blocks.pop();
    }

    if (block.getDepth() % base::config::BC_DIFFICULTY_RECALCULATION_RATE != 0) {
        return;
    }

    const Block& p = *_last_blocks.front();

    auto elapsed = (block.getTimestamp() - p.getTimestamp()).getSeconds();
    ASSERT(elapsed);
    if constexpr (!base::config::IS_DEBUG) {
        if (elapsed == 0) {
            elapsed = 1;
        }
    }

    static constexpr int TARGET =
      base::config::BC_DIFFICULTY_RECALCULATION_RATE * 60 / base::config::BC_TARGET_BLOCKS_PER_MINUTE;

    if (TARGET >= 4 * elapsed) {
        _complexity = Complexity{ _complexity.getDensed() / 4 };
    }
    else if (4 * TARGET <= elapsed) { // TARGET
        _complexity = Complexity{ _complexity.getDensed() * 4 };
    }
    else {
        _complexity = Complexity{ _complexity.getDensed() * elapsed / TARGET };
    }
}
}