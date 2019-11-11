#include "hash.hpp"

#include "base/assert.hpp"

#include <openssl/sha.h>

#include <sstream>
#include <iomanip>
#include <iostream>

namespace base
{

base::Bytes sha256(const base::Bytes& data)
{
    char digest[SHA256_DIGEST_LENGTH + 1];
    digest[SHA256_DIGEST_LENGTH] = '\0';
    SHA256(data.toArray(), data.size(), reinterpret_cast<unsigned char*>(&digest));
    base::Bytes ret(digest);
    ASSERT(ret.size() == 32);
    return ret;
}


base::Bytes sha1(const base::Bytes& data)
{
    char digest[SHA_DIGEST_LENGTH + 1];
    digest[SHA_DIGEST_LENGTH] = '\0';
    SHA1(data.toArray(), data.size(), reinterpret_cast<unsigned char*>(&digest));
    base::Bytes ret(digest);
    ASSERT(ret.size() == 20);
    return ret;
}

}