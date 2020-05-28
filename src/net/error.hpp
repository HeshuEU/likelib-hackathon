#pragma once

#include "base/error.hpp"

namespace net
{

struct Error : base::Error
{
    using base::Error::Error;
};


struct ClosedSession : Error
{
    using Error::Error;
};


struct SendOnClosedConnection : Error
{
    using Error::Error;
};

} // namespace net
