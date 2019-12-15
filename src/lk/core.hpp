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
    void tryAddBlock(const bc::Block& b);
    bc::Balance getBalance(const bc::Address& address) const;
    //==================
    void performTransaction(const bc::Transaction& tx);
    //==================
    const bc::Block& getTopBlock() const;
    //==================
  private:
    //==================
    const base::PropertyTree& _config;
    //==================
    BalanceManager _balance_manager;
    bc::Blockchain _blockchain;
    lk::ProtocolEngine _protocol_engine;
    //==================
    static const bc::Block& getGenesisBlock();
    void applyGenesis();
    //==================
    bool checkBlock(const bc::Block& block) const;
    //==================
};

} // namespace lk
