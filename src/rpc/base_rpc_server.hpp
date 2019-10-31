#pragma once

#include "base/address.hpp"
#include "base/types.hpp"

#include <string>

class BaseServer
{
  public:
    virtual base::BalanceIntType balance(const base::Address& address) = 0;
    virtual std::string transaction(base::BalanceIntType amount, const base::Address& from_address,
                                    const base::Address& to_address) = 0;
};
