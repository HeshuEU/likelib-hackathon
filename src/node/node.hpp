#pragma once

#include "miner.hpp"
#include "public_service.hpp"

#include "core/core.hpp"

#include "base/crypto.hpp"
#include "base/property_tree.hpp"

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
    PublicService _rpc;
    //---------------------------
    std::unique_ptr<Miner> _miner;
    //---------------------------
    void onBlockMine(lk::ImmutableBlock&& block);
    void onNewTransactionReceived(const lk::Transaction& tx);
    void onNewBlock(const lk::ImmutableBlock& block);
};