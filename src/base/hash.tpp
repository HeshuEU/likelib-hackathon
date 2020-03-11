#include "hash.hpp"

#include <openssl/ripemd.h>

namespace base
{
template<std::size_t S>
Sha256 Sha256::compute(const FixedBytes<S>& data)
{
    base::FixedBytes<SHA256_SIZE> ret;
    SHA256(data.toArray(), S, ret.toArray());
    return Sha256(ret);
}


template<std::size_t S>
Sha1 Sha1::compute(const FixedBytes<S>& data)
{
    base::FixedBytes<SHA1_SIZE> ret;
    SHA1(data.toArray(), S, reinterpret_cast<unsigned char*>(ret.toArray()));
    return Sha1(ret);
}


template<std::size_t S>
Ripemd160 Ripemd160::compute(const FixedBytes<S>& data)
{
    base::FixedBytes<RIPEMD160_SIZE> ret;
    RIPEMD160_CTX context;
    if(1 != RIPEMD160_Init(&context)) {
        RAISE_ERROR(CryptoError, "failed to initialize context for Ripemd160");
    }

    if(1 != RIPEMD160_Update(&context, data.toArray(), S)) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }

    if(1 != RIPEMD160_Final(ret.toArray(), &context)) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }
    return Ripemd160(ret);
}
} // namespace base