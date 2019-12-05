#include "connection.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "net/error.hpp"
#include "net/packet.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <atomic>
#include <thread>
#include <utility>

namespace ba = boost::asio;

namespace net
{


base::Bytes Connection::_read_buffer(base::config::NET_MESSAGE_BUFFER_SIZE);


Connection::Connection(
    boost::asio::io_context& io_context, boost::asio::ip::tcp::socket&& socket, ReceiveHandler receive_handler)
    : _id{getNextId()}, _io_context{io_context}, _socket{std::move(socket)}, _receive_handler(receive_handler)
{
    ASSERT(_socket.is_open());
    _connect_endpoint =
        std::make_unique<Endpoint>(_socket.remote_endpoint().address().to_string(), _socket.remote_endpoint().port());
}


Connection::~Connection()
{
    if(!_is_closed) {
        try {
            close();
        }
        catch(const std::exception& e) {
            LOG_WARNING << "Error while closing connection: " << e.what();
        }
    }
}


std::size_t Connection::getNextId()
{
    static std::atomic<int> next_id{0};
    return next_id++;
}


std::size_t Connection::getId() const noexcept
{
    return _id;
}


void Connection::close()
{
    // we could have just return if the connection is already closed, but it helps a lot to catch bugs during debug
    ASSERT_SOFT(!_is_closed);
    _is_closed = true;
    if(_socket.is_open()) {
        LOG_INFO << "Shutting down connection to " << _connect_endpoint->toString();
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


bool Connection::isClosed() const noexcept
{
    return _is_closed;
}


const Endpoint& Connection::getEndpoint() const
{
    return *_connect_endpoint;
}


bool Connection::hasServerEndpoint() const noexcept
{
    return _server_endpoint.get() != nullptr;
}


const Endpoint& Connection::getServerEndpoint() const
{
    if(_server_endpoint) {
        return *_server_endpoint;
    }
    else {
        RAISE_ERROR(net::Error, "connection doesn't have server endpoint");
    }
}


void Connection::setServerEndpoint(const Endpoint& server_endpoint)
{
    ASSERT(!_server_endpoint);
    _server_endpoint = std::make_unique<Endpoint>(server_endpoint);
}


void Connection::startReceivingMessages()
{
    ASSERT_SOFT(!_is_receiving_enabled);
    _is_receiving_enabled = true;
    receiveOne();
}


void Connection::stopReceivingMessages()
{
    ASSERT_SOFT(_is_receiving_enabled);
    _is_receiving_enabled = false;
}


void Connection::receiveOne()
{
    // ba::transfer_at_least just for now for debugging purposes, of course will be changed later
    ba::async_read(_socket, ba::buffer(_read_buffer.toVector()), ba::transfer_at_least(5),
        [this, cp = shared_from_this()](const boost::system::error_code& ec, const std::size_t bytes_received) {
            if(_is_closed) {
                LOG_DEBUG << "Received on closed connection";
                return;
            }
            else if(ec) {
                switch(ec.value()) {
                    case ba::error::eof: {
                        LOG_WARNING << "Connection to " << getEndpoint() << " closed";
                        if(!_is_closed) {
                            close();
                        }
                        break;
                    }
                    default: {
                        LOG_WARNING << "Error occurred while receiving: " << ec << ' ' << ec.message();
                        break;
                    }
                }
                // TODO: do something
            }
            else {
                // LOG_DEBUG << "Received " << bytes_received << " bytes from " << _connect_endpoint;

                if(_is_receiving_enabled) {
                    try {
                        auto packet = base::fromBytes<Packet>(_read_buffer);
                        _receive_handler(*this, std::move(packet));
                    }
                    catch(const std::exception& e) {
                        LOG_WARNING << "Error during packet handling: " << e.what();
                    }

                    // double-check since the value may be changed - we don't know how long the handler was executing
                    if(_is_receiving_enabled) {
                        receiveOne();
                    }
                }
            }
        });
}


void Connection::send(const Packet& packet)
{
    LOG_DEBUG << "SEND [" << enumToString(packet.getType()) << ']';
    send(base::toBytes(packet));
}


void Connection::send(base::Bytes&& data)
{
    bool is_already_writing = !_pending_send_messages.empty();
    _pending_send_messages.push(std::move(data));

    if(!is_already_writing) {
        sendPendingMessages();
    }
}


void Connection::sendPendingMessages()
{
    ASSERT_SOFT(!_pending_send_messages.empty());
    if(_pending_send_messages.empty()) {
        return;
    }

    base::Bytes& message = _pending_send_messages.front();
    ba::async_write(_socket, ba::buffer(message.toVector()),
        [this, cp = shared_from_this()](const boost::system::error_code& ec, const std::size_t bytes_sent) {
            if(_is_closed) {
                return;
            }
            else if(ec) {
                LOG_WARNING << "Error while sending message: " << ec << ' ' << ec.message();
                // TODO: do something
            }
            else {
                // LOG_DEBUG << "Sent " << bytes_sent << " bytes to " << _connect_endpoint->toString();
            }
            _pending_send_messages.pop();

            if(!_pending_send_messages.empty()) {
                sendPendingMessages();
            }
        });
}


void Connection::startSession()
{
    startReceivingMessages();
}

} // namespace net
