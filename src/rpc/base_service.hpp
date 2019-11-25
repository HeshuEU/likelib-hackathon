#pragma once

#include "bc/address.hpp"
#include "bc/types.hpp"

#include <string>

namespace rpc
{

class BaseService
{
  public:
    virtual ~BaseService() = default;

    virtual bc::Balance balance(const bc::Address& address) = 0;

    virtual std::string transaction(
        bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address) = 0;

    virtual std::string test(const std::string& test_request) = 0;
};

} // namespace rpc
