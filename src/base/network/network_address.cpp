#include "network_address.hpp"

#include "base/error.hpp"

#include <boost/spirit/include/qi.hpp>

#include <iterator>

namespace base::network
{

NetworkAddress::NetworkAddress(const std::string_view& address_with_port)
{
    std::size_t i = address_with_port.find(':');
    if(i == std::string_view::npos) {
        RAISE_ERROR(InvalidArgument, "port is not specified");
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
        RAISE_ERROR(InvalidArgument, std::string{"invalid address "} + std::string{address_with_port});
    }
}

NetworkAddress::NetworkAddress(const std::string_view& address, std::size_t port)
{}


std::string NetworkAddress::toString()
{
    return _address.to_string() + ":" + std::to_string(_port);
}


NetworkAddress::operator boost::asio::ip::address_v4()
{
    return _address;
}


unsigned short NetworkAddress::getPort() const noexcept
{
    return _port;
}


NetworkAddress::operator boost::asio::ip::tcp::endpoint()
{
    return {_address, _port};
}


} // namespace base::network