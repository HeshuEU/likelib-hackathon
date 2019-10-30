#pragma once

#include <cassert>

#define CHECK(condition, message) \
    do { \
        if(!(condition)) { \
            assert(false); \
        } \
    } while(0)
