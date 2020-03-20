#include "connector.hpp"

#include "base/log.hpp"
#include "net/connection.hpp"
#include "net/session.hpp"

namespace ba = boost::asio;

namespace net
{

Connector::Connector(boost::asio::io_context& io_context)
  : _io_context{ io_context }
{}


void Connector::connect(const Endpoint& address, std::function<void(std::unique_ptr<Connection>)> on_connect)
{
    LOG_DEBUG << "Connecting to " << address.toString();
    auto socket = std::make_unique<ba::ip::tcp::socket>(_io_context);
    socket->async_connect(static_cast<ba::ip::tcp::endpoint>(address),
                          [this, socket = std::move(socket), address, on_connect = std::move(on_connect)](
                            const boost::system::error_code& ec) mutable {
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