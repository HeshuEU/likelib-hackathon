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
    //TODO:check string is base64
    if(base64_address.size() != ADDRESS_SIZE){
        RAISE_ERROR(base::InvalidArgument, "the invalid size of string for the Address");
    }
}


const std::string& Address::getNullAddressString()
{
    static const std::string null_address(ADDRESS_SIZE, '0');
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


bool Address::operator<(const Address& other) const
{
    return toString() < other.toString();
}


Address Address::deserialize(base::SerializationIArchive& ia)
{
    std::string address = ia.deserialize<std::string>();
    return Address(address);
}


void Address::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(toString());
}


std::ostream& operator<<(std::ostream& os, const Address& address)
{
    return os << address.toString();
}

} // namespace bc
