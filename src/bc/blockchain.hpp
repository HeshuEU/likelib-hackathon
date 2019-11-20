#pragma once

#include "bc/block.hpp"
#include "bc/transaction.hpp"


namespace bc
{


class Blockchain
{
  public:
    void blockReceived(Block&& block);
    void transactionReceived(Transaction&& transaction);

  private:
};

} // namespace bc