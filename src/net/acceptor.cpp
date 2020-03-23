#include "acceptor.hpp"

#include "base/log.hpp"

namespace ba = boost::asio;

namespace net
{

Acceptor::Acceptor(ba::io_context& io_context, const Endpoint& listen_endpoint)
  : _io_context{ io_context }
  , _listen_endpoint{ listen_endpoint }
  , _acceptor{ _io_context }
{
    _acceptor.open(static_cast<ba::ip::tcp::endpoint>(listen_endpoint).protocol());
    _acceptor.set_option(ba::socket_base::reuse_address(true));
    _acceptor.bind(static_cast<ba::ip::tcp::endpoint>(listen_endpoint));
    _acceptor.listen();
}


void Acceptor::accept(std::function<void(std::unique_ptr<Connection>)> on_accept)
{
    _acceptor.async_accept(
      [this, on_accept = std::move(on_accept)](const boost::system::error_code& ec, ba::ip::tcp::socket socket) {
          if (ec) {
              LOG_WARNING << "Connection accept failed: " << ec;
          }
          else {
              auto connection = std::make_unique<Connection>(_io_context, std::move(socket));
              LOG_INFO << "Connection accepted: " << connection->getEndpoint();
              if (on_accept) {
                  on_accept(std::move(connection));
              }
          }
      });
}


} // namespace net