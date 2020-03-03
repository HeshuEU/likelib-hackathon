#include "address.hpp"

namespace bc
{


Address::Address(const base::RsaPublicKey& pub)
{
    auto sha256 = base::Sha256::compute(pub.toBytes());
    auto ripemd = base::Ripemd160::compute(sha256.getBytes());
    ASSERT(ripemd.getBytes().size() == BYTE_LENGTH);
    _address = ripemd.getBytes();
}


Address::Address(const std::string_view& base64_address)
{
    _address = base::base64Decode(base64_address);
    if(_address.size() != BYTE_LENGTH) {
        RAISE_ERROR(base::InvalidArgument, "invalid base64 string");
    }
    // TODO: check address length
}


Address::Address(base::Bytes raw_address)
{
    if(raw_address.size() != BYTE_LENGTH) {
        RAISE_ERROR(base::InvalidArgument, "invalid bytes length");
    }
    _address = std::move(raw_address);
}


const Address& Address::null()
{
    static const Address null_address{base::Bytes(Address::BYTE_LENGTH)};
    return null_address;
}


bool Address::isNull() const
{
    return *this == null();
}


std::string Address::toString() const
{
    return base::base64Encode(_address);
}


const base::Bytes& Address::getBytes() const noexcept
{
    return _address;
}


bool Address::operator==(const Address& another) const
{
    return _address == another._address;
}


bool Address::operator!=(const Address& another) const
{
    return !(*this == another);
}


bool Address::operator<(const Address& other) const
{
    return toString() < other.toString();
}


Address Address::deserialize(base::SerializationIArchive& ia)
{
    auto address_bytes = ia.deserialize<base::Bytes>();
    return Address(address_bytes);
}


void Address::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_address);
}


std::ostream& operator<<(std::ostream& os, const Address& address)
{
    return os << address.toString();
}

} // namespace bc
