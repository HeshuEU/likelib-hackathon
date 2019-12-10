#pragma once

#include "base/property_tree.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "net/host.hpp"


namespace lk
{

class Core
{
  public:
    //==================
    Core(const base::PropertyTree& config);
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
    bc::Blockchain _blockchain;
    net::Host _host;
    //==================
    void broadcastBlock(const bc::Block& block);
    //==================
};

} // namespace lk
