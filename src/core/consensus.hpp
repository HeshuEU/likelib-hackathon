#pragma once

#include "base/big_integer.hpp"
#include "base/bytes.hpp"
#include "core/block.hpp"

#include <queue>

namespace lk
{

class Complexity
{
    static constexpr std::size_t LENGTH = base::Sha256::LENGTH; // bytes
  public:
    static const Complexity& minimal();

    using Comparer = base::FixedBytes<LENGTH>;
    using Densed = base::Uint256;

    explicit Complexity(Densed densed);
    const Comparer& getComparer() const noexcept;
    const Densed& getDensed() const noexcept;

  private:
    Densed _densed;
    Comparer _comparer;

    static Comparer calcComparer(const Densed& densed);
};


class Consensus
{
  public:
    explicit Consensus();

    bool checkBlock(const ImmutableBlock& block) const;

    void applyBlock(ImmutableBlock block);

    const Complexity& getComplexity() const;

  private:
    std::queue<ImmutableBlock> _last_blocks;
    Complexity _complexity;
};

}