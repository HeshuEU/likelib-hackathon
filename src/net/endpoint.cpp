#include "endpoint.hpp"

#include "base/error.hpp"

#include <boost/spirit/include/qi.hpp>

#include <iterator>
#include <ostream>

namespace net
{

Endpoint::Endpoint(const std::string_view& address_with_port)
{
    std::size_t i = address_with_port.find(':');
    if (i == std::string_view::npos) {
        RAISE_ERROR(base::InvalidArgument, "port is not specified");
    }

    std::string_view ip_address_part = address_with_port.substr(0, i);
    std::string_view port_part = address_with_port.substr(i + 1, address_with_port.length() - (i + 1));

    try {
        _address = boost::asio::ip::make_address_v4(ip_address_part);
        if (!boost::spirit::qi::parse(std::cbegin(port_part), std::cend(port_part), boost::spirit::qi::int_, _port)) {
            RAISE_ERROR(base::InvalidArgument, std::string{ address_with_port });
        }
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InvalidArgument, std::string{ address_with_port });
    }
}

Endpoint::Endpoint(const std::string_view& address, std::size_t port)
{
    _address = boost::asio::ip::make_address_v4(address);
    _port = port;
}


std::string Endpoint::toString() const
{
    return _address.to_string() + ":" + std::to_string(_port);
}


Endpoint::operator boost::asio::ip::address_v4() const
{
    return _address;
}


unsigned short Endpoint::getPort() const noexcept
{
    return _port;
}


void Endpoint::setPort(unsigned short port) noexcept
{
    _port = port;
}


Endpoint::operator boost::asio::ip::tcp::endpoint() const
{
    return { _address, _port };
}


bool Endpoint::operator==(const Endpoint& other) const
{
    return _address == other._address && _port == other._port;
}


bool Endpoint::operator!=(const Endpoint& other) const
{
    return !(*this == other);
}


bool Endpoint::operator<(const Endpoint& other) const
{
    return _address < other._address || (_address == other._address && _port < other._port);
}


bool Endpoint::operator>(const Endpoint& other) const
{
    return _address > other._address || (_address == other._address && _port > other._port);
}


bool Endpoint::operator<=(const Endpoint& other) const
{
    return !(*this > other);
}


bool Endpoint::operator>=(const Endpoint& other) const
{
    return !(*this < other);
}


std::ostream& operator<<(std::ostream& os, const Endpoint& endpoint)
{
    return os << endpoint.toString();
}


Endpoint Endpoint::deserialize(base::SerializationIArchive& ia)
{
    std::string ip_with_port = ia.deserialize<std::string>();
    return { ip_with_port };
}


void Endpoint::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(toString());
}

} // namespace net