#include "address.hpp"

namespace bc
{


Address::Address() : _address{getNullAddressString()}
{}


Address Address::fromPublicKey(const base::RsaPublicKey& pub)
{
    auto sha256 = base::Sha256::compute(pub.toBytes());
    auto ripemd = base::Ripemd160::compute(sha256.getBytes());
    return Address{base::base64Encode(ripemd.getBytes())};
}


Address::Address(const std::string_view& base64_address) : _address{base64_address}
{
    // TODO: check address length
}


const std::string& Address::getNullAddressString()
{
    static const std::string null_address(32, '0');
    return null_address;
}


bool Address::isNull() const
{
    return _address == getNullAddressString();
}


std::string Address::toString() const
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


bool operator<(const Address& a, const Address& b)
{
    return a.toString() < b.toString();
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Address& tx)
{
    std::string address = ia.deserialize<std::string>();
    tx = Address(address);
    return ia;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Address& tx)
{
    oa.serialize(tx.toString());
    return oa;
}


std::ostream& operator<<(std::ostream& os, const Address& address)
{
    return os << address.toString();
}

} // namespace bc
