#include "address.hpp"

namespace bc
{

Address::Address() : _hash(base::Sha256::calcSha256(base::Bytes()))
{}

Address::Address(const base::Sha256& hash) : _hash(hash)
{}

Address::Address(const std::string& data_string) : _hash(data_string)
{}

std::string Address::toString()
{
    return _hash.toString();
}

bool Address::operator==(Address& another)
{
    return _hash == another._hash;
}

bool Address::operator==(const Address& another)
{
    return _hash == another._hash;
}

bool operator<(const Address& another_1, const Address& another_2)
{
    return another_1._hash.getBytes() < another_2._hash.getBytes();
}

} // namespace bc
