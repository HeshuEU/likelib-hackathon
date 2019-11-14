#pragma once

#include "base/error.hpp"

namespace network
{

struct Error : base::Error
{
    using base::Error::Error;
};

}
