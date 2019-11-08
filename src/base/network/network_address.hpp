#pragma once

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cstddef>
#include <string_view>


namespace base::network
{

// TODO: think about naming of this class
class NetworkAddress
{
  public:
    NetworkAddress(const std::string_view& address_with_port);
    NetworkAddress(const std::string_view& address, std::size_t port);

    std::string toString();

    std::size_t getPort() const noexcept;

    operator boost::asio::ip::address_v4();

    operator boost::asio::ip::tcp::endpoint();

  private:
    boost::asio::ip::address_v4 _address;
    std::size_t _port;
};

} // namespace base::network
