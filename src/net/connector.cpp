#include "connector.hpp"

#include <boost/asio/steady_timer.hpp>

#include "base/log.hpp"
#include "net/connection.hpp"

namespace ba = boost::asio;

namespace net
{

Connector::ConnectError::ConnectError(Status status, boost::system::error_code ec)
  : _status{ status }
  , _ec{ ec }
{}


Connector::ConnectError::Status Connector::ConnectError::getStatus() const noexcept
{
    return _status;
}


boost::system::error_code Connector::ConnectError::getErrorCode() const
{
    return _ec;
}


Connector::Connector(boost::asio::io_context& io_context)
  : _io_context{ io_context }
{}


namespace
{

struct ConnectEventStatus
{
    ConnectEventStatus(std::size_t timeout_seconds,
                       ba::io_context& context,
                       std::function<void(Connector::ConnectError)> on_fail)
      : on_fail{ std::move(on_fail) }
      , deadline{ context }
    {
        deadline.expires_after(std::chrono::seconds(timeout_seconds));
    }

    bool is_already_connected{ false };
    std::function<void(Connector::ConnectError)> on_fail;
    ba::steady_timer deadline;
};

}


void Connector::connect(const Endpoint& address,
                        std::size_t timeout_seconds,
                        std::function<void(std::unique_ptr<Connection>)> on_connect,
                        std::function<void(ConnectError)> on_fail)
{
    LOG_DEBUG << "Connecting to " << address.toString();
    auto socket = std::make_unique<ba::ip::tcp::socket>(_io_context);
    auto event_status = std::make_shared<ConnectEventStatus>(timeout_seconds, _io_context, std::move(on_fail));

    event_status->deadline.async_wait([event_status](const boost::system::error_code&) {
        if (!event_status->is_already_connected) {
            event_status->on_fail(ConnectError{ Connector::ConnectError::Status::TIMEOUT });
        }
    });

    socket->async_connect(static_cast<ba::ip::tcp::endpoint>(address),
                          [this, socket = std::move(socket), address, on_connect = std::move(on_connect), event_status](
                            const boost::system::error_code& ec) mutable {
                              event_status->is_already_connected = true;
                              event_status->deadline.cancel();

                              if (ec) {
                                  switch (ec.value()) {
                                      case ba::error::connection_refused: {
                                          LOG_WARNING << "Connection error: host " << address.toString();
                                          break;
                                      }
                                      case ba::error::fault: {
                                          LOG_WARNING << "Connection error: invalid address";
                                          break;
                                      }
                                      default: {
                                          LOG_WARNING << "Connection error: " << ec << ' ' << ec.message();
                                          break;
                                      }
                                  }
                                  event_status->on_fail(ConnectError{ ConnectError::Status::NETWORK_FAILURE, ec });
                              }
                              else {
                                  auto connection =
                                    std::make_unique<Connection>(_io_context, std::move(*socket.release()));
                                  LOG_INFO << "Connection established: " << connection->getEndpoint();
                                  if (on_connect) {
                                      on_connect(std::move(connection));
                                  }
                              }
                          });
}

} // namespace net