#pragma once

#include "base/config.hpp"

#ifdef CONFIG_IS_DEBUG
    #define CHECK(condition, message) \
        throw base::Error{message}
#else
    #define CHECK(condition, message) do {} while(0)
#endif