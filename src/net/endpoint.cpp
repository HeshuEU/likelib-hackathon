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
    if(i == std::string_view::npos) {
        RAISE_ERROR(base::InvalidArgument, "port is not specified");
    }

    std::string_view ip_address_part = address_with_port.substr(0, i);
    std::string_view port_part = address_with_port.substr(i + 1, address_with_port.length() - (i + 1));

    try {
        _address = boost::asio::ip::make_address_v4(ip_address_part);
        if(!boost::spirit::qi::parse(std::cbegin(port_part), std::cend(port_part), boost::spirit::qi::int_, _port)) {
            throw base::InvalidArgument{};
        }
    }
    catch(const std::exception& e) {
        RAISE_ERROR(base::InvalidArgument, std::string{"invalid address "} + std::string{address_with_port});
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


Endpoint::operator boost::asio::ip::tcp::endpoint() const
{
    return {_address, _port};
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


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Endpoint& endpoint)
{
    std::string ip_with_port;
    ia >> ip_with_port;
    endpoint = Endpoint(ip_with_port);
    return ia;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Endpoint& endpoint)
{
    return oa << endpoint.toString();
}

} // namespace net