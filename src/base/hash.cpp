#include "hash.hpp"

#include "base/assert.hpp"

namespace base
{

base::Bytes sha256(const base::Bytes& data)
{
    base::Bytes ret{256 / 8};

    ASSERT(ret.size() == 32);
    return ret;
}


base::Bytes sha1(const base::Bytes& data)
{
    base::Bytes ret(160 / 8);

    ASSERT(ret.size() == 20);
    return ret;
}

}