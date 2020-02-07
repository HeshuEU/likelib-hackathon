#pragma once

#include "base/property_tree.hpp"
#include "lk/core.hpp"
#include "node/miner.hpp"
#include "rpc/rpc.hpp"
#include "node/rpc_service.hpp"
#include "base/crypto.hpp"

class Node
{
  public:
    explicit Node(const base::PropertyTree& config);
    void run();

  private:
    const base::PropertyTree& _config;
    //---------------------------
    base::KeyVault _key_vault;
    //---------------------------
    lk::Core _core;
    std::unique_ptr<rpc::RpcServer> _rpc;
    //---------------------------
    std::unique_ptr<Miner> _miner;
    //---------------------------
    static base::Bytes getMiningComplexity();
    //---------------------------
    void onBlockMine(bc::Block&& block);
    void onNewTransactionReceived(const bc::Transaction& tx);
    void onNewBlock(const bc::Block& block);
};