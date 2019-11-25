#pragma once

#include "base/property_tree.hpp"
#include "bc/balance_manager.hpp"
#include "bc/block.hpp"
#include "bc/miner.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"
#include "net/network.hpp"

#include <list>
#include <optional>

namespace bc
{


class Blockchain
{
  public:
    //===================
    explicit Blockchain(const base::PropertyTree& config);
    //===================
    void run();
    //===================
    void processReceivedBlock(Block&& block);
    void processReceivedTransaction(Transaction&& transaction);
    //===================
    void addBlock(const Block& block);
    //===================
    bc::Balance getBalance(const bc::Address& address) const;
    //===================
  private:
    //===================
    std::list<Block> _blocks;
    TransactionsSet _pending_txs;
    Block _pending_block;
    Miner _miner;
    std::unique_ptr<net::Network> _network;
    bc::BalanceManager _balance_manager;
    const base::PropertyTree& _config;
    //===================
    void setupGenesis();
    void onMinerFinished(const std::optional<Block>& block);
};

} // namespace bc