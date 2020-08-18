#include "address.hpp"

namespace lk
{


Address::Address(const base::FixedBytes<SIGNATURE_LENGTH>& sign)
{
    auto sha256 = base::Sha256::compute(sign.toBytes());
    auto ripemd = base::Ripemd160::compute(sha256.getBytes());
    _address = ripemd.getBytes();
}


Address::Address(const std::string_view& base58_address)
  : _address(base::base58Decode(base58_address))
{}


Address::Address(const base::Bytes& raw_address)
  : _address(raw_address)
{}


Address::Address(const base::FixedBytes<Address::LENGTH_IN_BYTES>& raw_address)
  : _address(raw_address)
{}


const Address& Address::null()
{
    static const Address null_address{ base::FixedBytes<Address::LENGTH_IN_BYTES>() };
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


const base::FixedBytes<Address::LENGTH_IN_BYTES>& Address::getBytes() const noexcept
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
    return Address(ia.deserialize<base::FixedBytes<LENGTH_IN_BYTES>>());
}


void Address::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_address);
}


std::ostream& operator<<(std::ostream& os, const Address& address)
{
    return os << address.toString();
}

} // namespace lk


std::size_t std::hash<lk::Address>::operator()(const lk::Address& k) const
{
    return std::hash<base::FixedBytes<lk::Address::LENGTH_IN_BYTES>>{}(k.getBytes());
}
