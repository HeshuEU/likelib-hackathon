#include "connection.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <utility>

namespace ba = boost::asio;

namespace base::network
{

Connection::Connection(boost::asio::ip::tcp::socket&& socket)
    : _io_context{socket.get_io_context()}, _socket{std::move(socket)}
{
    ASSERT(_socket.is_open());
    _network_address = std::make_unique<NetworkAddress>(_socket.remote_endpoint().address().to_string(),
                                                        _socket.remote_endpoint().port());

    _receiveOne();
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


void Connection::_receiveOne()
{
    //    boost::asio::async_read(
    //        _socket, boost::asio::buffer(_read_buffer.toArray(), _read_buffer.size()),
    //        std::bind(&Connection::_receiveHandler, this, std::placeholders::_1, std::placeholders::_2));
}


void Connection::send(base::Bytes&& data)
{
    bool is_already_writing = !_write_pending_messages.empty();
    _write_pending_messages.push(std::move(data));

    if(!is_already_writing) {
        _sendPendingMessages();
    }
}


void Connection::_sendPendingMessages()
{
    ASSERT_SOFT(!_write_pending_messages.empty());
    if(_write_pending_messages.empty()) {
        return;
    }

    base::Bytes& message = _write_pending_messages.front();
    ba::async_write(_socket, boost::asio::buffer(message.toArray(), message.size()),
                    std::bind(&Connection::_sendHandler, this, std::placeholders::_1, std::placeholders::_2));
}


void Connection::_sendHandler(const boost::system::error_code& ec, std::size_t bytes_sent)
{
    if(ec) {
        LOG_WARNING << "Error while sending message: " << ec;
        // TODO: do something
    }

    LOG_INFO << "Sent " << bytes_sent << " bytes to " << _network_address->toString();
    _write_pending_messages.pop();

    if(!_write_pending_messages.empty()) {
        _sendPendingMessages();
    }
}


void Connection::_receiveHandler(const boost::system::error_code& error, std::size_t bytes_received)
{
    LOG_INFO << "Received " << bytes_received << " bytes from " << _network_address->toString();
    if(_on_receive) {
        ReadHandler& user_handler = *_on_receive;
        // user_handler(_read_buffer);
    }
    _receiveOne();
}


void Connection::setOnReceive(Connection::ReadHandler handler)
{
    _on_receive = std::make_unique<ReadHandler>(std::move(handler));
}


} // namespace base::network
