#pragma once

#include "net/connection.hpp"
#include "net/endpoint.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <functional>

namespace net
{

class Connector
{
  public:
    //===================
    class ConnectError
    {
      public:
        enum class Status
        {
            SUCCESS,
            TIMEOUT,
            NETWORK_FAILURE,
        };

        Status getStatus() const noexcept;
        boost::system::error_code getErrorCode() const;

      private:
        Status _status;
        boost::system::error_code _ec;

        ConnectError(Status status, boost::system::error_code ec = {});

        friend Connector;
    };

    //===================
    explicit Connector(boost::asio::io_context& io_context);
    //===================
    void connect(const Endpoint& address,
                 std::size_t timeout_seconds,
                 std::function<void(std::unique_ptr<Connection>)> on_connect,
                 std::function<void(ConnectError)> on_fail);
    //===================
  private:
    //===================
    boost::asio::io_context& _io_context;
    //===================
};

} // namespace net