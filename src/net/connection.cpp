#include "connection.hpp"

#include "base/assert.hpp"
#include "base/config.hpp"
#include "base/log.hpp"
#include "net/error.hpp"
#include "net/packet.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <thread>
#include <utility>

namespace ba = boost::asio;

namespace net
{

Connection::Connection(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket&& socket)
    : _io_context{io_context}, _socket{std::move(socket)}, _read_buffer(base::config::NET_MESSAGE_BUFFER_SIZE)
{
    ASSERT(_socket.is_open());
    const auto& re = _socket.remote_endpoint();
    _connect_endpoint = std::make_unique<Endpoint>(re.address().to_string(), re.port());
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


void Connection::receive(std::size_t bytes_to_receive, net::Connection::ReceiveHandler receive_handler)
{
    ba::async_read(_socket, ba::buffer(_read_buffer.toVector()), ba::transfer_exactly(bytes_to_receive),
        [this, cp = shared_from_this(), handler = std::move(receive_handler)](
            const boost::system::error_code& ec, const std::size_t bytes_received) mutable {
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
                try {
                    _read_buffer.resize(bytes_received);
                    (std::move(handler))(_read_buffer);
                    _read_buffer.resize(_read_buffer.capacity());
                }
                catch(const std::exception& e) {
                    LOG_WARNING << "Error during packet handling: " << e.what();
                }
            }
        });
}


void Connection::send(base::Bytes data)
{
    LOG_DEBUG << "SENDING [" << data.size() << "bytes ]";

    bool is_already_writing;
    {
        std::lock_guard lk(_pending_send_messages_mutex);
        is_already_writing = !_pending_send_messages.empty();

        _pending_send_messages.push(std::move(data));
    }

    if(!is_already_writing) {
        sendPendingMessages();
    }
}


void Connection::sendPendingMessages()
{
    std::lock_guard lk(_pending_send_messages_mutex);
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

            std::lock_guard lk(_pending_send_messages_mutex);
            _pending_send_messages.pop();

            if(!_pending_send_messages.empty()) {
                sendPendingMessages();
            }
        });
}

} // namespace net
