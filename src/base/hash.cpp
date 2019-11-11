#include "hash.hpp"

#include "base/assert.hpp"

#include <openssl/sha.h>

#include <sstream>
#include <iomanip>

namespace base
{

base::Bytes sha256(const base::Bytes& data)
{
    base::Bytes ret(32);
    SHA256(data.toArray(), data.size(), reinterpret_cast<unsigned char*>(ret.toArray()));
    ASSERT(ret.size() == 32);
    return ret;
}


base::Bytes sha1(const base::Bytes& data)
{
    base::Bytes ret(20);
    SHA1(data.toArray(), data.size(), reinterpret_cast<unsigned char*>(ret.toArray()));
    ASSERT(ret.size() == 20);
    return ret;
}

}