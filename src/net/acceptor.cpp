#include "acceptor.hpp"

#include "base/log.hpp"

namespace ba = boost::asio;

namespace net
{

Acceptor::Acceptor(ba::io_context& io_context, const Endpoint& listen_endpoint, Peers& accepted_peers)
    : _io_context{io_context}, _listen_endpoint{listen_endpoint},
      _acceptor{_io_context, static_cast<ba::ip::tcp::endpoint>(listen_endpoint)}, _accepted_peers{accepted_peers}
{
    _acceptor.set_option(ba::socket_base::reuse_address(true));
}


void Acceptor::acceptInfinite(std::function<void(Peer&)> on_accept)
{
    ASSERT(_is_stopped);
    _is_stopped = false;
    _on_accept = std::move(on_accept);
    acceptOneImpl();
    LOG_INFO << "Listening on " << _listen_endpoint.toString();
}


void Acceptor::stopAccept()
{
    ASSERT_SOFT(!_is_stopped);
    _is_stopped = true;
}


void Acceptor::acceptOneImpl()
{
    if(_is_stopped) {
        return;
    }

    _acceptor.async_accept([this](const boost::system::error_code& ec, ba::ip::tcp::socket socket) {
        if(_is_stopped) {
            return;
        }
        else if(ec) {
            LOG_WARNING << "Connection accept failed: " << ec;
        }
        else {
            auto connection = std::make_unique<Connection>(_io_context, std::move(socket));
            LOG_INFO << "Connection accepted: " << connection->getEndpoint();
            auto peer = std::make_shared<Peer>(std::move(connection));
            _accepted_peers.add(peer);
            if(_on_accept) {
                _on_accept(*peer);
            }
        }

        if(_is_stopped) {
            acceptOneImpl();
        }
    });
}


} // namespace net