#include "hash.hpp"

#include <openssl/evp.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>

namespace base
{

template<std::size_t S>
Sha256 Sha256::compute(const FixedBytes<S>& data)
{
    base::FixedBytes<LENGTH> ret;
    SHA256(data.getData(), S, ret.getData());
    return Sha256(ret);
}


template<std::size_t S>
Sha1 Sha1::compute(const FixedBytes<S>& data)
{
    base::FixedBytes<LENGTH> ret;
    SHA1(data.getData(), S, reinterpret_cast<unsigned char*>(ret.getData()));
    return Sha1(ret);
}


template<std::size_t S>
Ripemd160 Ripemd160::compute(const FixedBytes<S>& data)
{
    base::FixedBytes<LENGTH> ret;
    RIPEMD160_CTX context;
    if (1 != RIPEMD160_Init(&context)) {
        RAISE_ERROR(CryptoError, "failed to initialize context for Ripemd160");
    }

    if (1 != RIPEMD160_Update(&context, data.getData(), S)) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }

    if (1 != RIPEMD160_Final(ret.getData(), &context)) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }
    return Ripemd160(ret);
}

} // namespace base