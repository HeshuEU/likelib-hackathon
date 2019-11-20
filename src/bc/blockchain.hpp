#pragma once

#include "bc/block.hpp"
#include "bc/transaction.hpp"

#include <list>

namespace bc
{


class Blockchain
{
  public:
    void blockReceived(Block&& block);
    void transactionReceived(Transaction&& transaction);

  private:
    std::list<Block> _blocks;
};

} // namespace bc