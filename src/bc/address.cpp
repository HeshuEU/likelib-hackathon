#include "address.hpp"

namespace bc
{


Address::Address(const base::RsaPublicKey& pub)
{
    auto sha256 = base::Sha256::compute(pub.toBytes());
    auto ripemd = base::Ripemd160::compute(sha256.getBytes());
    _address = ripemd.getBytes();
}


Address::Address(const std::string_view& base58_address) : _address(base::base58Decode(base58_address))
{}


Address::Address(const base::Bytes& raw_address) : _address(raw_address)
{}


Address::Address(const base::FixedBytes<Address::ADDRESS_BYTES_LENGTH>& raw_address) : _address(raw_address)
{}


const Address& Address::null()
{
    static const Address null_address{base::FixedBytes<Address::ADDRESS_BYTES_LENGTH>()};
    return null_address;
}


bool Address::isNull() const
{
    return *this == null();
}


std::string Address::toString() const
{
    return base::base58Encode(_address.toBytes());
}


const base::FixedBytes<Address::ADDRESS_BYTES_LENGTH>& Address::getBytes() const noexcept
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
    return Address(ia.deserialize<base::FixedBytes<ADDRESS_BYTES_LENGTH>>());
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
