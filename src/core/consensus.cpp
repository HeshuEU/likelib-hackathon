#include "consensus.hpp"

#include "base/config.hpp"

namespace lk
{

Complexity::Comparer Complexity::calcComparer(const Complexity::Densed& densed)
{
    Complexity::Comparer ret;
    std::size_t index = LENGTH;
    for (auto d = densed; d > 0; d /= 256) {
        ret[--index] = (d % 256).convert_to<base::Byte>();
    }
    return ret;
}


Complexity::Complexity(Complexity::Densed densed)
  : _densed{ std::move(densed) }
  , _comparer{ calcComparer(_densed) }
{}


const Complexity& Complexity::minimal()
{
    static const auto ret = [] {
        base::Uint256 v = (base::Uint256{ 1 } << 252) + (base::Uint256{ 1 } << 251);
        return Complexity{ v };
    }();
    return ret;
}


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


bool Consensus::checkBlock(const ImmutableBlock& block) const
{
    return block.getHash().getBytes() <= _complexity.getComparer();
}


void Consensus::applyBlock(ImmutableBlock block)
{
    _last_blocks.push(std::move(block)); // TODO: change it, of course
    if (_last_blocks.size() < base::config::BC_DIFFICULTY_RECALCULATION_RATE) {
        // means we do not have enough block to recalculate anything
        return;
    }

    if (_last_blocks.size() > base::config::BC_DIFFICULTY_RECALCULATION_RATE) {
        _last_blocks.pop();
    }

    if (block.getDepth() != 0 && (block.getDepth() - 1) % base::config::BC_DIFFICULTY_RECALCULATION_RATE != 0) {
        return;
    }

    const ImmutableBlock& p = _last_blocks.front();

    auto elapsed = (block.getTimestamp() - p.getTimestamp()).getSeconds();
    ASSERT(elapsed);
    if constexpr (!base::config::IS_DEBUG) {
        if (elapsed == 0) {
            elapsed = 1;
        }
    }

    static constexpr int TARGET =
      base::config::BC_DIFFICULTY_RECALCULATION_RATE * 60 / base::config::BC_TARGET_BLOCKS_PER_MINUTE;


    double r = double(elapsed) / TARGET;
    if (r < 1) {
        r = std::max(r, 1. / base::config::BC_DIFFICULTY_RECALCULATION_RATE);
        base::Uint256 m = std::clamp(static_cast<std::size_t>(std::round(1 / r)),
                                     std::size_t{ 1 },
                                     base::config::BC_DIFFICULTY_RECALCULATION_RATE);
        _complexity = Complexity{ _complexity.getDensed() / m };
    }
    else {
        r = std::min(r, static_cast<double>(base::config::BC_DIFFICULTY_RECALCULATION_RATE));
        base::Uint256 m = std::min(~base::Uint256{} / _complexity.getDensed(),
                                   base::Uint256{ base::config::BC_DIFFICULTY_RECALCULATION_RATE });
        std::size_t limit = m.convert_to<double>();
        base::Uint256 mult = std::clamp(static_cast<std::size_t>(std::round(r)), std::size_t{}, limit);
        _complexity = Complexity{ _complexity.getDensed() * mult };
    }
}

}