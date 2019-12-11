#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "lkcore/balance_manager.hpp"
#include "net/host.hpp"


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
    ~Core();
    //==================
    /**
     *  @brief Loads blockchain from disk and runs networking.
     *
     *  @async
     *  @threadsafe
     */
    void run();
    //==================
  private:
    //==================
    const base::PropertyTree& _config;
    //==================
    BalanceManager _balance_manager;
    bc::Blockchain _blockchain;
    net::Host _host;
    //==================
    void broadcastBlock(const bc::Block& block);
    //==================
    static const bc::Block& getGenesisBlock();
    void applyGenesis();
    //==================
};

} // namespace lk
