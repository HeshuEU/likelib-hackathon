#pragma once

#include "base/crypto.hpp"
#include "base/property_tree.hpp"
#include "core/core.hpp"
#include "node/miner.hpp"
#include "node/rpc_service.hpp"
#include "rpc/rpc.hpp"

class Node
{
  public:
    explicit Node(const base::PropertyTree& config);
    void run();

  private:
    //---------------------------
    const base::PropertyTree& _config;
    //---------------------------
    base::KeyVault _key_vault;
    //---------------------------
    lk::Core _core;
    std::unique_ptr<rpc::BaseRpcServer> _rpc;
    //---------------------------
    std::unique_ptr<Miner> _miner;
    //---------------------------
    static base::FixedBytes<impl::CommonData::COMPLEXITY_SIZE> getMiningComplexity();
    //---------------------------
    void onBlockMine(lk::Block&& block);
    void onNewTransactionReceived(const lk::Transaction& tx);
    void onNewBlock(const lk::Block& block);
};