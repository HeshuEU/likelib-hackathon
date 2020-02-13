#include "address.hpp"

namespace bc
{

const Address BASE_ADDRESS{"00000000000000000000000000000000"};


Address::Address(const char* data_string) : _address(base::Bytes::fromHex(data_string))
{}


Address::Address(base::Sha256 hash) : _address(std::move(hash))
{}


Address::Address(const std::string& data_string) : _address(base::Bytes::fromHex(data_string))
{}


std::string Address::toString() const
{
    return _address.toHex();
}


bool Address::operator==(const Address& another) const
{
    return _address == another._address;
}


bool Address::operator<(const Address& another) const
{
    return toString() < another.toString();
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
