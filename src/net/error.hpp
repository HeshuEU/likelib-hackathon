#pragma once

#include "base/error.hpp"

namespace net
{

struct Error : base::Error
{
    using base::Error::Error;
};

}
