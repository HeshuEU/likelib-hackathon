#include "address.hpp"

namespace bc
{

// const Address BASE_ADDRESS{"00000000000000000000000000000000"};


Address::Address(const base::RsaPublicKey& pub)
{
    auto sha256 = base::Sha256::compute(pub.toBytes());
    auto ripemd = base::Ripemd160::compute(sha256.getBytes());
    _address = base::base64Encode(ripemd.getBytes());
}


std::string Address::toString() const
{
    return _address;
}


bool Address::operator==(const Address& another) const
{
    return _address == another._address;
}


bool Address::operator<(const Address& another) const
{
    return toString() < another.toString();
}


Address Address::deserialize(base::SerializationIArchive& ia)
{
    auto pub = base::RsaPublicKey::deserialize(ia);
    return bc::Address{pub};
}


void Address::serialize(base::SerializationOArchive& oa) const
{
    oa << _public_key;
}


std::ostream& operator<<(std::ostream& os, const Address& address)
{
    return os << address.toString();
}

} // namespace bc
