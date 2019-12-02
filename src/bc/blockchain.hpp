#pragma once

#include "base/property_tree.hpp"
#include "bc/balance_manager.hpp"
#include "bc/block.hpp"
#include "bc/miner.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"
#include "net/network.hpp"

#include <list>

namespace bc
{

class Blockchain
{
  public:
    //===================
    explicit Blockchain(const base::PropertyTree& config);
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    ~Blockchain() = default;
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
    bool _is_running{false};
    //===================
    std::list<Block> _blocks;
    mutable std::recursive_mutex _blocks_mutex;
    //===================
    Block _pending_block;
    mutable std::recursive_mutex _pending_block_mutex;
    //===================
    Miner _miner;
    //===================
    bc::BalanceManager _balance_manager;
    mutable std::recursive_mutex _balance_manager_mutex;
    //===================
    const base::PropertyTree& _config;
    //===================
    std::unique_ptr<net::Network> _network;

    class NetworkHandler : public net::NetworkHandler
    {
      public:
        NetworkHandler(Blockchain&);
        void onBlockReceived(Block&& block) override;
        void onTransactionReceived(Transaction&& tx) override;

      private:
        Blockchain& _bc;
    };

    NetworkHandler _network_handler;
    //===================
    void setupGenesis();
    //===================
    base::Bytes getMiningComplexity() const;
    void onMinerFinished(Block block);
    //===================
    bool checkBlock(const Block& block) const;
    bool checkTransaction(const Transaction& tx) const;
    //===================
};

} // namespace bc