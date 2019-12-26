#include "hash.hpp"

#include "base/assert.hpp"
#include "error.hpp"

#include <openssl/sha.h>


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


SerializationOArchive& Sha256::serialize(SerializationOArchive& oa, const Sha256& block)
{
    return block.serialize(oa);
}


SerializationOArchive& Sha256::serialize(SerializationOArchive& oa) const
{
    return oa << _bytes;
}


Sha256 Sha256::deserialize(SerializationIArchive& ia)
{
    Bytes data;
    ia >> data;
    return Sha256(data);
}


std::ostream& operator<<(std::ostream& os, const Sha256& sha)
{
    return os << sha.getBytes().toHex();
}


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
    return oa << _bytes;
}


Sha1 Sha1::deserialize(SerializationIArchive& ia)
{
    Bytes data;
    ia >> data;
    return Sha1(data);
}


std::ostream& operator<<(std::ostream& os, const Sha1& sha)
{
    return os << sha.getBytes().toHex();
}

} // namespace base


std::size_t std::hash<base::Sha256>::operator()(const base::Sha256& k) const
{
    return std::hash<base::Bytes>{}(k.getBytes());
}


std::size_t std::hash<base::Sha1>::operator()(const base::Sha1& k) const
{
    return std::hash<base::Bytes>{}(k.getBytes());
}