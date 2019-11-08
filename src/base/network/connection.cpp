#include "connection.hpp"

#include "base/log.hpp"

#include <utility>

namespace ba = boost::asio;

namespace base::network
{

Connection::Connection(boost::asio::ip::tcp::socket&& socket) : _socket{std::move(socket)}
{
    _network_address = std::make_unique<NetworkAddress>(_socket.remote_endpoint().address().to_string(),
                                                        _socket.remote_endpoint().port());
}

Connection::~Connection()
{
    if(_socket.is_open()) {
        LOG_INFO << "Shutting down connection to " << _network_address->toString();
        boost::system::error_code ec;
        _socket.shutdown(ba::ip::tcp::socket::shutdown_both, ec);
        if(ec) {
            LOG_WARNING << "Error occurred while shutting down connection: " << ec.message();
        }
        _socket.close(ec);
        if(ec) {
            LOG_WARNING << "Error occurred while closing connection: " << ec.message();
        }
    }
}

const NetworkAddress& Connection::getRemoteNetworkAddress() const
{
    return *_network_address;
}

} // namespace base::network