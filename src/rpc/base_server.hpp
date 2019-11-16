#pragma once

#include "bc/address.hpp"
#include "bc/types.hpp"

#include <string>

namespace rpc
{

class BaseServer
{
  public:
    virtual ~BaseServer() = default;

    virtual bc::Balance balance(const bc::Address& address) = 0;

    virtual std::string transaction(
        bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address) = 0;
};

} // namespace rpc
