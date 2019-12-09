#include "connector.hpp"

#include "base/log.hpp"

namespace ba = boost::asio;

namespace net
{

Connector::Connector(boost::asio::io_context& io_context, Peers& peers) : _io_context{io_context}, _peers{peers}
{}


void Connector::connect(const Endpoint& address, std::function<void(Peer&)> on_connect)
{
    LOG_DEBUG << "Connecting to " << address.toString();
    auto socket = std::make_unique<ba::ip::tcp::socket>(_io_context);
    socket->async_connect(static_cast<ba::ip::tcp::endpoint>(address),
        [this, socket = std::move(socket), address, on_connect](const boost::system::error_code& ec) mutable {
            if(ec) {
                switch(ec.value()) {
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
                auto connection = std::make_unique<Connection>(_io_context, std::move(*socket.release()));
                LOG_INFO << "Connection established: " << connection->getEndpoint();
                auto peer = std::make_shared<Peer>(std::move(connection));
                _peers.add(peer);
                if(on_connect) {
                    on_connect(*peer);
                }
            }
        });
}

} // namespace net