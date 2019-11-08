#pragma once

#include "base/bytes.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace base::network
{

class Connection
{
  public:
    Connection(boost::asio::io_context& io_context);

    void send(const base::Bytes&);

    boost::asio::ip::tcp::socket socket;

  private:
};


} // namespace base::network