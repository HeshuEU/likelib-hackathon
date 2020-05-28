#include "connection.hpp"

#include "base/assert.hpp"
#include "base/config.hpp"
#include "base/log.hpp"
#include "net/error.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <thread>
#include <utility>

namespace ba = boost::asio;

namespace net
{

Connection::Connection(boost::asio::io_context& io_context,
                       boost::asio::ip::tcp::socket&& socket,
                       CloseHandler close_handler)
  : _io_context{ io_context }
  , _socket{ std::move(socket) }
  , _close_handler{ std::move(close_handler) }
  , _read_buffer(base::config::NET_MESSAGE_BUFFER_SIZE)
{
    ASSERT(_socket.is_open());
    const auto& re = _socket.remote_endpoint();
    _connect_endpoint = std::make_unique<Endpoint>(re.address().to_string(), re.port());
}


Connection::~Connection()
{
    if (!_is_closed) {
        try {
            close();
        }
        catch (const std::exception& e) {
            LOG_WARNING << "Error while closing connection: " << e.what();
        }
    }
}


void Connection::close()
{
    // we could have just return if the connection is already closed, but it helps a lot to catch bugs during debug
    ASSERT_SOFT(!_is_closed);
    _is_closed = true;
    if (_socket.is_open()) {
        boost::system::error_code ec;
        _socket.shutdown(ba::ip::tcp::socket::shutdown_both, ec);
        if (ec) {
            LOG_WARNING << "Error occurred while shutting down connection: " << ec.message();
        }
        _socket.close(ec);
        if (ec) {
            LOG_WARNING << "Error occurred while closing connection: " << ec.message();
        }
    }

    if (_close_handler) {
        std::lock_guard lk(_close_handler_mutex);
        _close_handler();
    }
}


bool Connection::isClosed() const noexcept
{
    return _is_closed;
}


void Connection::setCloseHandler(CloseHandler handler)
{
    std::lock_guard lk(_close_handler_mutex);
    _close_handler = std::move(handler);
}


const Endpoint& Connection::getEndpoint() const
{
    return *_connect_endpoint;
}


void Connection::receive(std::size_t bytes_to_receive, net::Connection::ReceiveHandler receive_handler)
{
    ba::async_read(_socket,
                   ba::buffer(_read_buffer.toVector()),
                   ba::transfer_exactly(bytes_to_receive),
                   [connection_holder = weak_from_this(), handler = std::move(receive_handler)](
                     const boost::system::error_code& ec, const std::size_t bytes_received) mutable {
                       if (auto connection = connection_holder.lock()) {
                           if (connection->_is_closed) {
                               LOG_DEBUG << "Received on closed connection";
                               return;
                           }
                           else if (ec) {
                               switch (ec.value()) {
                                   case ba::error::eof:
                                   case ba::error::connection_reset: {
                                       LOG_WARNING << "Connection to " << connection->getEndpoint() << " closed";
                                       if (!connection->_is_closed) {
                                           connection->close();
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
                               try {
                                   //_read_buffer.resize(bytes_received);
                                   (std::move(handler))(connection->_read_buffer);
                                   connection->_read_buffer.resize(base::config::NET_MESSAGE_BUFFER_SIZE);
                               }
                               catch (const std::exception& e) {
                                   LOG_WARNING << "Error during packet handling: " << e.what();
                               }
                           }
                       }
                   });
}


void Connection::send(base::Bytes data)
{
    bool is_already_writing;
    {
        std::lock_guard lk(_pending_send_messages_mutex);
        is_already_writing = !_pending_send_messages.empty();

        _pending_send_messages.push({ std::move(data), {} });
    }

    if (!is_already_writing) {
        sendPendingMessages();
    }
}


void Connection::send(base::Bytes data, Connection::SendHandler send_handler)
{
    bool is_already_writing;
    {
        std::lock_guard lk(_pending_send_messages_mutex);
        is_already_writing = !_pending_send_messages.empty();

        _pending_send_messages.push({ std::move(data), std::move(send_handler) });
    }

    if (!is_already_writing) {
        sendPendingMessages();
    }
}


void Connection::sendPendingMessages()
{
    std::lock_guard lk(_pending_send_messages_mutex);
    ASSERT_SOFT(!_pending_send_messages.empty());
    if (_pending_send_messages.empty()) {
        return;
    }

    auto& data = _pending_send_messages.front();
    auto& message = data.first;
    auto& callback = data.second;

    ba::async_write(_socket,
                    ba::buffer(message.toVector()),
                    [connection_holder = weak_from_this(), callback](const boost::system::error_code& ec,
                                                                     const std::size_t bytes_sent) {
                        if (auto connection = connection_holder.lock()) {
                            if (connection->_is_closed) {
                                return;
                            }
                            else if (ec) {
                                LOG_WARNING << "Error while sending message: " << ec << ' ' << ec.message();
                                // TODO: do something, check if connection is dropped
                            }
                            else {
                                LOG_DEBUG << "Sent " << bytes_sent << " bytes to "
                                          << connection->_connect_endpoint->toString();
                            }

                            if (callback) {
                                callback();
                            }

                            std::lock_guard lk(connection->_pending_send_messages_mutex);
                            connection->_pending_send_messages.pop();

                            if (!connection->_pending_send_messages.empty()) {
                                connection->sendPendingMessages();
                            }
                        }
                    });
}

} // namespace net
