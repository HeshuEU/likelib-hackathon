#pragma once

#include <string>

class BaseRpcServer
{
  public:
    virtual int balance(const std::string& address) = 0;

    virtual std::string transaction(int amount, const std::string& from_address, const std::string& to_address) = 0;
};
