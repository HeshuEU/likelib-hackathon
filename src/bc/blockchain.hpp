#pragma once

#include "bc/block.hpp"
#include "bc/miner.hpp"
#include "bc/transaction.hpp"

#include <list>

namespace net
{
class Network;
}

namespace bc
{


class Blockchain
{
  public:
    //===================
    Blockchain();

    //===================
    void blockReceived(Block&& block);
    void transactionReceived(Transaction&& transaction);

    void addBlock(const Block& block);

    void setNetwork(net::Network* network);

  private:
    std::list<Block> _blocks;
    Block _pending_block;
    Miner _miner;

    net::Network* _network{nullptr};
};

} // namespace bc