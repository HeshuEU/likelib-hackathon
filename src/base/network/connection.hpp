#pragma once

#include "base/bytes.hpp"

#include <boost/asio/io_context.hpp>

namespace base::network
{

class Connection
{
  public:
    Connection(boost::asio::io_context& io_context);

    void send(const base::Bytes&);

  private:
};


} // namespace base::network