#pragma once

#include "status.hpp"

#include <utility>

namespace base
{


template<typename... Args>
Status Status::Error(Args&&... args)
{
    return Status{base::Error(std::forward<Args>(args)...)};
}


}