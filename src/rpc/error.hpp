#pragma once

#include "base/error.hpp"

namespace rpc
{

class RpcError : public ::base::Error
{
    using ::base::Error::Error;
};

} // namespace rpc