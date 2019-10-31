#pragma once

#include "base/types.hpp"

#include <string>

class BaseRpcServer
{
  public:
    virtual base::BalanceIntType balance(const std::string& address) = 0;

    virtual std::string transaction(int amount, const std::string& from_address, const std::string& to_address) = 0;
};
