#pragma once

#include "base/error.hpp"

namespace web_socket
{

struct Error : base::Error
{
    using base::Error::Error;
};


struct SetUpError : Error
{
    using Error::Error;
};


struct ClosedSession : Error
{
    using Error::Error;
};


struct SendOnClosedConnection : Error
{
    using Error::Error;
};

} // namespace web_socket