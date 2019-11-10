#include "connection.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <utility>

namespace ba = boost::asio;

namespace network
{


base::Bytes Connection::_read_buffer(base::config::NETWORK_MESSAGE_BUFFER_SIZE);


Connection::Connection(boost::asio::ip::tcp::socket&& socket)
    : _io_context{socket.get_io_context()}, _socket{std::move(socket)}
{
    ASSERT(_socket.is_open());
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


void Connection::startReceivingMessages()
{
    ASSERT_SOFT(!_is_receiving_enabled);
    _is_receiving_enabled = true;
    _receiveOne();
}


void Connection::stopReceivingMessages()
{
    ASSERT_SOFT(_is_receiving_enabled);
    _is_receiving_enabled = false;
}


void Connection::_receiveOne()
{
    // ba::transfer_at_least just for now for debugging purposes, of course will be changed later
    ba::async_read(_socket, ba::buffer(_read_buffer.toVector()), ba::transfer_at_least(5),
                   std::bind(&Connection::_receiveHandler, this, std::placeholders::_1, std::placeholders::_2));
}


void Connection::send(base::Bytes&& data)
{
    bool is_already_writing = !_pending_send_messages.empty();
    _pending_send_messages.push(std::move(data));

    if(!is_already_writing) {
        _sendPendingMessages();
    }
}


void Connection::_sendPendingMessages()
{
    ASSERT_SOFT(!_pending_send_messages.empty());
    if(_pending_send_messages.empty()) {
        return;
    }

    base::Bytes& message = _pending_send_messages.front();
    ba::async_write(_socket, ba::buffer(message.toVector()),
                    std::bind(&Connection::_sendHandler, this, std::placeholders::_1, std::placeholders::_2));
}


void Connection::_sendHandler(const boost::system::error_code& ec, std::size_t bytes_sent)
{
    if(ec) {
        LOG_WARNING << "Error while sending message: " << ec;
        // TODO: do something
    }

    LOG_INFO << "Sent " << bytes_sent << " bytes to " << _network_address->toString();
    _pending_send_messages.pop();

    if(!_pending_send_messages.empty()) {
        _sendPendingMessages();
    }
}


void Connection::_receiveHandler(const boost::system::error_code& ec, std::size_t bytes_received)
{
    if(ec) {
        LOG_WARNING << "Error occurred while receiving: " << ec;
    }
    else {
        LOG_INFO << "Received " << bytes_received << " bytes from " << _network_address->toString();
    }
    if(_is_receiving_enabled) {
        if(_on_receive) {
            ReadHandler& user_handler = *_on_receive;
            user_handler(_read_buffer);
        }

        // double-check since the value may be changed - we don't know how long the handler was executing
        if(_is_receiving_enabled) {
            _receiveOne();
        }
    }
}


void Connection::setOnReceive(Connection::ReadHandler handler)
{
    _on_receive = std::make_unique<ReadHandler>(std::move(handler));
}


void Connection::startSession()
{
    setOnReceive([](const base::Bytes& message) {
        LOG_INFO << "Received: " << reinterpret_cast<const char*>(message.toArray());
    });
    startReceivingMessages();


    send({48, 49, 50, 68, 65, 78});
}


} // namespace network
