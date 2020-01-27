#include "address.hpp"

namespace bc
{

const Address BASE_ADDRESS{"00000000000000000000000000000000"};


Address::Address(const char* data_string) : _address(std::string(data_string))
{}


Address::Address() : _address{}
{}


// Address::Address(const base::Sha256& hash) : _hash(hash)
// {}


Address::Address(const std::string& data_string) : _address(data_string)
{}


std::string Address::toString() const
{
    return _address.toHex();
}


bool Address::operator==(const Address& another) const
{
    return _address == another._address;
}


bool operator<(const Address& a, const Address& b)
{
    return a.toString() < b.toString();
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Address& tx)
{
    std::string address;
    ia >> address;
    tx = Address(address);
    return ia;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Address& tx)
{
    return oa << tx.toString();
}


std::ostream& operator<<(std::ostream& os, const Address& address)
{
    return os << address.toString();
}

} // namespace bc
