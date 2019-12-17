#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "lk/balance_manager.hpp"
#include "lk/protocol.hpp"
#include "net/host.hpp"

#include <boost/signals2.hpp>

namespace lk
{

class Core
{
  public:
    //==================
    Core(const base::PropertyTree& config);

    /**
     *  @brief Stops network, does cleaning and flushing.
     *
     *  @threadsafe
     */
    ~Core() = default;
    //==================
    /**
     *  @brief Loads blockchain from disk and runs networking.
     *
     *  @async
     *  @threadsafe
     */
    void run();
    //==================
    boost::signals2::signal<void(const bc::Block& block)> signal_new_block;
    boost::signals2::signal<void(const bc::Transaction& transaction)> signal_new_transaction;
    //==================
    bc::Balance getBalance(const bc::Address& address) const;
    //==================
    bool performTransaction(const bc::Transaction& tx);
    //==================
    bool tryAddBlock(const bc::Block& b);
    std::optional<bc::Block> findBlock(const base::Sha256& hash) const;
    const bc::Block& getTopBlock() const;
    //==================
    bc::Block getBlockTemplate() const;
    //==================
  private:
    //==================
    const base::PropertyTree& _config;
    //==================
    bool _is_balance_manager_updated{false};
    BalanceManager _balance_manager;
    bc::Blockchain _blockchain;
    lk::Network _network;
    bc::TransactionsSet _pending_transactions;
    //==================
    static const bc::Block& getGenesisBlock();
    void applyGenesis();
    void loadBlockchainFromDisk();
    void updateNewBlock(const bc::Block& block);
    //==================
    bool checkBlock(const bc::Block& block) const;
    //==================
};

} // namespace lk
