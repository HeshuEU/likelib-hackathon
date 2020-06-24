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
    RpcService _rpc;
    //---------------------------
    std::unique_ptr<Miner> _miner;
    //---------------------------
    static base::FixedBytes<impl::CommonData::COMPLEXITY_SIZE> getMiningComplexity();
    //---------------------------
    void onBlockMine(lk::Block&& block);
    void onNewTransactionReceived(const lk::Transaction& tx);
    void onNewBlock(const base::Sha256& block_hash, const lk::Block& block);
};