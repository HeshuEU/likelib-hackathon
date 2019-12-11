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
    base::PropertyTree _config;
    lk::Core _core;
    Miner _miner;
};