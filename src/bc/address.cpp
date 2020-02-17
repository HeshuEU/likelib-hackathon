#include "address.hpp"

namespace bc
{

// const Address BASE_ADDRESS{"00000000000000000000000000000000"};


Address::Address(std::string_view hex) : _public_key{base::Bytes::fromHex(hex)}
{}


Address::Address(base::RsaPublicKey pub) : _public_key{std::move(pub)}
{}


std::string Address::toString() const
{
    return _public_key.toBytes().toHex();
}


const base::RsaPublicKey& Address::getPublicKey() const
{
    return _public_key;
}


bool Address::operator==(const Address& another) const
{
    return _public_key.toBytes() == another._public_key.toBytes();
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
