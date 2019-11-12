#include "hash.hpp"

#include "base/assert.hpp"

#include <openssl/sha.h>

#include <sstream>
#include <iomanip>

namespace base
{

base::Bytes sha256(const base::Bytes& data)
{
    base::Bytes ret(SHA256_DIGEST_LENGTH);
    SHA256(data.toArray(), data.size(), reinterpret_cast<unsigned char*>(ret.toArray())); //reinterpret_cast is necessary if base::Byte changes
    ASSERT(ret.size() == SHA256_DIGEST_LENGTH);
    return ret;
}


base::Bytes sha1(const base::Bytes& data)
{
    base::Bytes ret(SHA_DIGEST_LENGTH);
    SHA1(data.toArray(), data.size(), reinterpret_cast<unsigned char*>(ret.toArray()));
    ASSERT(ret.size() == SHA_DIGEST_LENGTH);
    return ret;
}

}
