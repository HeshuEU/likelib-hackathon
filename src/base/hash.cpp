#include "hash.hpp"

#include "base/assert.hpp"
#include "error.hpp"

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/ripemd.h>


namespace base
{

Sha256::Sha256(const Bytes& data) : _bytes(data)
{
    if(_bytes.size() != SHA256_DIGEST_LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha256::Sha256(Bytes&& data) : _bytes(data)
{
    if(_bytes.size() != SHA256_DIGEST_LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


std::string Sha256::toHex() const
{
    return _bytes.toHex();
}


const base::Bytes& Sha256::getBytes() const noexcept
{
    return _bytes;
}


Sha256 Sha256::fromHex(const std::string& hex_view)
{
    auto bytes = Bytes::fromHex(hex_view);
    return Sha256(bytes);
}


bool Sha256::operator==(const Sha256& another) const
{
    return getBytes() == another.getBytes();
}


bool Sha256::operator!=(const Sha256& another) const
{
    return getBytes() != another.getBytes();
}


Sha256 Sha256::compute(const base::Bytes& data)
{
    base::Bytes ret(SHA256_DIGEST_LENGTH);
    SHA256(data.toArray(), data.size(), ret.toArray());
    ASSERT(ret.size() == SHA256_DIGEST_LENGTH);
    return Sha256(ret);
}


SerializationOArchive& Sha256::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
    return oa;
}


Sha256 Sha256::deserialize(SerializationIArchive& ia)
{
    Bytes data = ia.deserialize<base::Bytes>();
    return Sha256(data);
}


std::ostream& operator<<(std::ostream& os, const Sha256& sha)
{
    return os << sha.getBytes().toHex();
}

} // namespace base


std::size_t std::hash<base::Sha256>::operator()(const base::Sha256& k) const
{
    return std::hash<base::Bytes>{}(k.getBytes());
}

namespace base
{
Sha1::Sha1(const Bytes& data) : _bytes(data)
{
    if(_bytes.size() != SHA_DIGEST_LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha1::Sha1(Bytes&& data) : _bytes(data)
{
    if(_bytes.size() != SHA_DIGEST_LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


std::string Sha1::toHex() const
{
    return _bytes.toHex();
}


const base::Bytes& Sha1::getBytes() const noexcept
{
    return _bytes;
}


Sha1 Sha1::fromHex(const std::string_view& hex_view)
{
    auto bytes = Bytes::fromHex(hex_view);
    return Sha1(bytes);
}


bool Sha1::operator==(const Sha1& another) const
{
    return getBytes() == another.getBytes();
}


bool Sha1::operator!=(const Sha1& another) const
{
    return getBytes() != another.getBytes();
}


Sha1 Sha1::compute(const base::Bytes& data)
{
    base::Bytes ret(SHA_DIGEST_LENGTH);
    SHA1(data.toArray(), data.size(), reinterpret_cast<unsigned char*>(ret.toArray()));
    ASSERT(ret.size() == SHA_DIGEST_LENGTH);
    return Sha1(ret);
}


SerializationOArchive& Sha1::serialize(SerializationOArchive& oa, const Sha1& block)
{
    return block.serialize(oa);
}


SerializationOArchive& Sha1::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
    return oa;
}


Sha1 Sha1::deserialize(SerializationIArchive& ia)
{
    Bytes data = ia.deserialize<base::Bytes>();
    return Sha1(data);
}


std::ostream& operator<<(std::ostream& os, const Sha1& sha)
{
    return os << sha.getBytes().toHex();
}

} // namespace base


std::size_t std::hash<base::Sha1>::operator()(const base::Sha1& k) const
{
    return std::hash<base::Bytes>{}(k.getBytes());
}


namespace base
{
Ripemd160::Ripemd160(const Bytes& data) : _bytes(data)
{
    if(_bytes.size() != RIPEMD160_DIGEST_LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size for Ripemd160");
    }
}


Ripemd160::Ripemd160(Bytes&& data) : _bytes(data)
{
    if(_bytes.size() != RIPEMD160_DIGEST_LENGTH) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size for Ripemd160");
    }
}


std::string Ripemd160::toHex() const
{
    return _bytes.toHex();
}


const base::Bytes& Ripemd160::getBytes() const noexcept
{
    return _bytes;
}


Ripemd160 Ripemd160::fromHex(const std::string& hex_view)
{
    auto bytes = Bytes::fromHex(hex_view);
    return Ripemd160(bytes);
}


bool Ripemd160::operator==(const Ripemd160& another) const
{
    return getBytes() == another.getBytes();
}


bool Ripemd160::operator!=(const Ripemd160& another) const
{
    return getBytes() != another.getBytes();
}


Ripemd160 Ripemd160::compute(const base::Bytes& data)
{
    base::Bytes ret(RIPEMD160_DIGEST_LENGTH);
    RIPEMD160_CTX context;
    if(1 != RIPEMD160_Init(&context)) {
        RAISE_ERROR(CryptoError, "failed to initialize context for Ripemd160");
    }

    if(1 != RIPEMD160_Update(&context, data.toArray(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }

    if(1 != RIPEMD160_Final(ret.toArray(), &context)) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }

    ASSERT(ret.size() == RIPEMD160_DIGEST_LENGTH);
    return Ripemd160(ret);
}


SerializationOArchive& Ripemd160::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
    return oa;
}


Ripemd160 Ripemd160::deserialize(SerializationIArchive& ia)
{
    Bytes data = ia.deserialize<base::Bytes>();
    return Ripemd160(data);
}


std::ostream& operator<<(std::ostream& os, const Ripemd160& ripemd)
{
    return os << ripemd.getBytes().toHex();
}

} // namespace base


std::size_t std::hash<base::Ripemd160>::operator()(const base::Ripemd160& k) const
{
    return std::hash<base::Bytes>{}(k.getBytes());
}


namespace base
{
Sha3::Sha3(const Bytes& data) : _type(getSha3Type(data.size())), _bytes(data)
{}


Sha3::Sha3(Bytes&& data) : _type(getSha3Type(data.size())), _bytes(data)
{}


std::string Sha3::toHex() const
{
    return _bytes.toHex();
}


const base::Bytes& Sha3::getBytes() const noexcept
{
    return _bytes;
}


Sha3::Sha3Type Sha3::getType() const noexcept
{
    return _type;
}


std::size_t Sha3::size() const noexcept
{
    return _bytes.size();
}


Sha3 Sha3::fromHex(const std::string_view& hex_view)
{
    auto bytes = Bytes::fromHex(hex_view);
    return Sha3(bytes);
}


bool Sha3::operator==(const Sha3& another) const
{
    return getBytes() == another.getBytes();
}


bool Sha3::operator!=(const Sha3& another) const
{
    return getBytes() != another.getBytes();
}


Sha3 Sha3::compute(const base::Bytes& data, Sha3::Sha3Type type)
{
    base::Bytes ret(static_cast<std::size_t>(type));
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), EVP_MD_CTX_free);

    switch(type) {
        case(Sha3::Sha3Type::Sha3Type224):
            if(1 != EVP_DigestInit_ex(context.get(), EVP_sha3_224(), NULL)) {
                RAISE_ERROR(CryptoError, "failed to initialize context");
            }
            break;
        case(Sha3::Sha3Type::Sha3Type256):
            if(1 != EVP_DigestInit_ex(context.get(), EVP_sha3_256(), NULL)) {
                RAISE_ERROR(CryptoError, "failed to initialize context");
            }
            break;
        case(Sha3::Sha3Type::Sha3Type384):
            if(1 != EVP_DigestInit_ex(context.get(), EVP_sha3_384(), NULL)) {
                RAISE_ERROR(CryptoError, "failed to initialize context");
            }
            break;
        case(Sha3::Sha3Type::Sha3Type512):
            if(1 != EVP_DigestInit_ex(context.get(), EVP_sha3_512(), NULL)) {
                RAISE_ERROR(CryptoError, "failed to initialize context");
            }
            break;
        default:
            RAISE_ERROR(InvalidArgument, "Sha3 type is not valid");
    }

    if(1 != EVP_DigestUpdate(context.get(), data.toArray(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to hash data");
    }

    std::unique_ptr<unsigned int> hash_length = std::make_unique<unsigned int>();
    if(1 != EVP_DigestFinal_ex(context.get(), ret.toArray(), hash_length.get())) {
        RAISE_ERROR(CryptoError, "failed to hash data");
    }

    ASSERT(ret.size() == *hash_length);
    return Sha3(ret);
}


Sha3::Sha3Type Sha3::getSha3Type(const std::size_t size)
{
    switch(size) {
        case(static_cast<std::size_t>(Sha3::Sha3Type::Sha3Type224)):
            return Sha3::Sha3Type::Sha3Type224;
            break;
        case(static_cast<std::size_t>(Sha3::Sha3Type::Sha3Type256)):
            return Sha3::Sha3Type::Sha3Type256;
            break;
        case(static_cast<std::size_t>(Sha3::Sha3Type::Sha3Type384)):
            return Sha3::Sha3Type::Sha3Type384;
            break;
        case(static_cast<std::size_t>(Sha3::Sha3Type::Sha3Type512)):
            return Sha3::Sha3Type::Sha3Type512;
            break;
        default:
            RAISE_ERROR(InvalidArgument, "bytes size for Sha3 is not valid");
    }
}


SerializationOArchive& Sha3::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
    return oa;
}


Sha3 Sha3::deserialize(SerializationIArchive& ia)
{
    Bytes data = ia.deserialize<base::Bytes>();
    return Sha3(data);
}


std::ostream& operator<<(std::ostream& os, const Sha3& sha)
{
    return os << sha.getBytes().toHex();
}

} // namespace base


std::size_t std::hash<base::Sha3>::operator()(const base::Sha3& k) const
{
    return std::hash<base::Bytes>{}(k.getBytes());
}
