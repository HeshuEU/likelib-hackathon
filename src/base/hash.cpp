#include "hash.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/ripemd.h>


namespace base
{

Sha256::Sha256(const Bytes& data) : _bytes(data)
{
    if(_bytes.size() != SHA256_SIZE) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha256::Sha256(Bytes&& data) : _bytes(data)
{
    if(_bytes.size() != SHA256_SIZE) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha256::Sha256(const FixedBytes<Sha256::SHA256_SIZE>& data) : _bytes(data)
{}


Sha256::Sha256(FixedBytes<Sha256::SHA256_SIZE>&& data) : _bytes(data)
{}


std::string Sha256::toHex() const
{
    return base::toHex<FixedBytes<SHA256_SIZE>>(_bytes);
}


const base::FixedBytes<Sha256::SHA256_SIZE>& Sha256::getBytes() const noexcept
{
    return _bytes;
}


Sha256 Sha256::null()
{
    return Sha256(base::Bytes(SHA256_SIZE));
}


Sha256 Sha256::fromHex(const std::string& hex_view)
{
    return Sha256(base::fromHex<Bytes>(hex_view));
}


bool Sha256::operator==(const Sha256& another) const
{
    return getBytes() == another.getBytes();
}


bool Sha256::operator!=(const Sha256& another) const
{
    return getBytes() != another.getBytes();
}


bool Sha256::operator<(const Sha256& another) const
{
    return getBytes() < another.getBytes();
}


Sha256 Sha256::compute(const base::Bytes& data)
{
    base::FixedBytes<SHA256_SIZE> ret;
    SHA256(data.getData(), data.size(), ret.getData());
    return Sha256(ret);
}


void Sha256::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Sha256 Sha256::deserialize(SerializationIArchive& ia)
{
    return Sha256(ia.deserialize<FixedBytes<SHA256_SIZE>>());
}


std::ostream& operator<<(std::ostream& os, const Sha256& sha)
{
    return os << toHex<FixedBytes<Sha256::SHA256_SIZE>>(sha.getBytes());
}

} // namespace base


std::size_t std::hash<base::Sha256>::operator()(const base::Sha256& k) const
{
    return std::hash<base::FixedBytes<base::Sha256::SHA256_SIZE>>{}(k.getBytes());
}

namespace base
{
Sha1::Sha1(const Bytes& data) : _bytes(data)
{
    if(_bytes.size() != SHA1_SIZE) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha1::Sha1(Bytes&& data) : _bytes(data)
{
    if(_bytes.size() != SHA1_SIZE) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size");
    }
}


Sha1::Sha1(const FixedBytes<Sha1::SHA1_SIZE>& data) : _bytes(data)
{}


Sha1::Sha1(FixedBytes<Sha1::SHA1_SIZE>&& data) : _bytes(data)
{}


std::string Sha1::toHex() const
{
    return base::toHex<FixedBytes<SHA1_SIZE>>(_bytes);
}


const base::FixedBytes<Sha1::SHA1_SIZE>& Sha1::getBytes() const noexcept
{
    return _bytes;
}


Sha1 Sha1::fromHex(const std::string_view& hex_view)
{
    return Sha1(base::fromHex<Bytes>(hex_view));
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
    base::FixedBytes<SHA1_SIZE> ret;
    SHA1(data.getData(), data.size(), reinterpret_cast<unsigned char*>(ret.getData()));
    return Sha1(ret);
}


void Sha1::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Sha1 Sha1::deserialize(SerializationIArchive& ia)
{
    return Sha1(ia.deserialize<base::FixedBytes<SHA1_SIZE>>());
}


std::ostream& operator<<(std::ostream& os, const Sha1& sha)
{
    return os << base::toHex<FixedBytes<Sha1::SHA1_SIZE>>(sha.getBytes());
}

} // namespace base


std::size_t std::hash<base::Sha1>::operator()(const base::Sha1& k) const
{
    return std::hash<base::FixedBytes<base::Sha1::SHA1_SIZE>>{}(k.getBytes());
}


namespace base
{
Ripemd160::Ripemd160(const Bytes& data) : _bytes(data)
{
    if(_bytes.size() != RIPEMD160_SIZE) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size for Ripemd160");
    }
}


Ripemd160::Ripemd160(Bytes&& data) : _bytes(data)
{
    if(_bytes.size() != RIPEMD160_SIZE) {
        RAISE_ERROR(InvalidArgument, "Not valid bytes size for Ripemd160");
    }
}


Ripemd160::Ripemd160(const FixedBytes<Ripemd160::RIPEMD160_SIZE>& data) : _bytes(data)
{}


Ripemd160::Ripemd160(FixedBytes<Ripemd160::RIPEMD160_SIZE>&& data) : _bytes(data)
{}


std::string Ripemd160::toHex() const
{
    return base::toHex<FixedBytes<Ripemd160::RIPEMD160_SIZE>>(_bytes);
}


const base::FixedBytes<Ripemd160::RIPEMD160_SIZE>& Ripemd160::getBytes() const noexcept
{
    return _bytes;
}


Ripemd160 Ripemd160::fromHex(const std::string& hex_view)
{
    return Ripemd160(base::fromHex<Bytes>(hex_view));
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
    base::FixedBytes<RIPEMD160_SIZE> ret;
    RIPEMD160_CTX context;
    if(1 != RIPEMD160_Init(&context)) {
        RAISE_ERROR(CryptoError, "failed to initialize context for Ripemd160");
    }

    if(1 != RIPEMD160_Update(&context, data.getData(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }

    if(1 != RIPEMD160_Final(ret.getData(), &context)) {
        RAISE_ERROR(CryptoError, "failed to hash data in Ripemd160");
    }
    return Ripemd160(ret);
}


void Ripemd160::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Ripemd160 Ripemd160::deserialize(SerializationIArchive& ia)
{
    return Ripemd160(ia.deserialize<base::FixedBytes<RIPEMD160_SIZE>>());
}


std::ostream& operator<<(std::ostream& os, const Ripemd160& ripemd)
{
    return os << ripemd.toHex();
}

} // namespace base


std::size_t std::hash<base::Ripemd160>::operator()(const base::Ripemd160& k) const
{
    return std::hash<base::FixedBytes<base::Ripemd160::RIPEMD160_SIZE>>{}(k.getBytes());
}


namespace base
{
Sha3::Sha3(const Bytes& data) : _type(getSha3Type(data.size())), _bytes(data)
{}


Sha3::Sha3(Bytes&& data) : _type(getSha3Type(data.size())), _bytes(data)
{}


std::string Sha3::toHex() const
{
    return base::toHex<Bytes>(_bytes);
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
    auto bytes = base::fromHex<Bytes>(hex_view);
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

    if(1 != EVP_DigestUpdate(context.get(), data.getData(), data.size())) {
        RAISE_ERROR(CryptoError, "failed to hash data");
    }

    std::unique_ptr<unsigned int> hash_length = std::make_unique<unsigned int>();
    if(1 != EVP_DigestFinal_ex(context.get(), ret.getData(), hash_length.get())) {
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


void Sha3::serialize(SerializationOArchive& oa) const
{
    oa.serialize(_bytes);
}


Sha3 Sha3::deserialize(SerializationIArchive& ia)
{
    Bytes data = ia.deserialize<base::Bytes>();
    return Sha3(data);
}


std::ostream& operator<<(std::ostream& os, const Sha3& sha)
{
    return os << toHex<Bytes>(sha.getBytes());
}

} // namespace base


std::size_t std::hash<base::Sha3>::operator()(const base::Sha3& k) const
{
    return std::hash<base::Bytes>{}(k.getBytes());
}
