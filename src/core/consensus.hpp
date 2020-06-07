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
    Complexity(base::Uint256 densed);
    const base::FixedBytes<LENGTH>& getComparer() const;
    const base::Uint256& getDensed() const noexcept;

  private:
    base::Uint256 _densed;
    base::FixedBytes<LENGTH> _comparer;

    static base::FixedBytes<LENGTH> calcComparer(const base::Uint256& densed);
};


class Consensus
{
  public:
    explicit Consensus();

    bool checkBlock(const Block& block) const;

    void applyBlock(const Block& block);

    const Complexity& getComplexity() const;

  private:
    std::queue<const Block*> _last_blocks;
    Complexity _complexity;
};

}