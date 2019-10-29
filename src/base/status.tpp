#pragma once

#include "status.hpp"

#include <utility>

namespace base
{


template<typename... Args>
Status Status::Error(Args&&... args)
{
    return Status::Error(base::Error(std::forward<Args>(args)...));
}


}