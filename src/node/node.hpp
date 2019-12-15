#pragma once

#include "base/property_tree.hpp"
#include "lk/core.hpp"
#include "node/miner.hpp"
#include "rpc/rpc.hpp"
#include "node/rpc_service.hpp"

class Node
{
  public:
    Node(const base::PropertyTree& config);
    void run();

  private:
    const base::PropertyTree& _config;
    lk::Core _core;
    std::unique_ptr<rpc::RpcServer> _rpc;

    std::unique_ptr<Miner> _miner;
    bc::Block _block_to_mine;

    void onBlockMine(bc::Block&& block);
    void onNewTransactionReceived(const bc::Transaction& tx);
    void onNewBlock(const bc::Block& block);
};