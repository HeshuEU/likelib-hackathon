#pragma once

#include "bc/balance_manager.hpp"
#include "bc/block.hpp"
#include "bc/miner.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"

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

    bc::Balance getBalance(const bc::Address& address) const;

  private:
    std::list<Block> _blocks;
    TransactionsSet _pending_txs;
    Block _pending_block;
    Miner _miner;
    net::Network* _network{nullptr};
    bc::BalanceManager _balance_manager;
};

} // namespace bc