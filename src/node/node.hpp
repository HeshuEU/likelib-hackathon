#pragma once

#include "miner.hpp"
//#include "public_service.hpp"

#include "core/core.hpp"

#include "base/crypto.hpp"

class Node
{
  public:
    explicit Node(rapidjson::Value config);
    void run();

  private:
    //---------------------------
    rapidjson::Value _config;
    //---------------------------
    lk::Core _core;
//    PublicService _public_service;
    //---------------------------
    std::unique_ptr<Miner> _miner;
    //---------------------------
    void onBlockMine(lk::ImmutableBlock&& block);
    void onNewTransactionReceived(const lk::Transaction& tx);
    void onNewBlock(const lk::ImmutableBlock& block);
};