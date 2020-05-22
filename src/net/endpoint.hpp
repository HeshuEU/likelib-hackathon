#pragma once

#include "base/serialization.hpp"

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cstddef>
#include <iosfwd>
#include <string_view>


namespace net
{

class Endpoint
{
  public:
    //=============
    Endpoint(const std::string_view& address_with_port);
    Endpoint(const std::string_view& address, std::size_t port);
    //=============
    std::string toString() const;

    operator boost::asio::ip::address_v4() const;
    operator boost::asio::ip::tcp::endpoint() const;
    //=============
    unsigned short getPort() const noexcept;
    void setPort(unsigned short port) noexcept;
    //=============
    bool operator==(const Endpoint& other) const;
    bool operator!=(const Endpoint& other) const;

    bool operator<(const Endpoint& other) const;
    bool operator>(const Endpoint& other) const;

    bool operator<=(const Endpoint& other) const;
    bool operator>=(const Endpoint& other) const;
    //=============
    static Endpoint deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=============
  private:
    //=============
    boost::asio::ip::address_v4 _address;

    // unsigned short is used in Boost to represent port number
    unsigned short _port;
    //=============
};


std::ostream& operator<<(std::ostream& os, const Endpoint& endpoint);

} // namespace net
