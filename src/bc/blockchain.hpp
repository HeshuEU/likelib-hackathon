#pragma once

#include "base/property_tree.hpp"
#include "bc/balance_manager.hpp"
#include "bc/block.hpp"
#include "bc/miner.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"
#include "net/host.hpp"

#include <list>
#include <shared_mutex>

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
    std::optional<bc::Block> findBlock(const base::Sha256& block_hash) const;
    //===================
    bc::Balance getBalance(const bc::Address& address) const;
    //===================
  private:
    //===================
    const base::PropertyTree& _config;
    //===================
    bool _is_running{false};
    //===================
    std::list<Block> _blocks;
    mutable std::shared_mutex _blocks_mutex;
    //===================
    Block _pending_block;
    mutable std::recursive_mutex _pending_block_mutex;
    //===================
    Miner _miner;
    //===================
    bc::BalanceManager _balance_manager;
    //===================
    void onNetworkReceived(base::Bytes&& data);
    //===================
    void setupGenesis();
    //===================
    base::Bytes getMiningComplexity() const;
    void onMinerFinished(Block&& block);
    //===================
    bool checkBlock(const Block& block) const;
    bool checkTransaction(const Transaction& tx) const;
    //===================
};

} // namespace bc