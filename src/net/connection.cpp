#include "connection.hpp"

#include "base/assert.hpp"
#include "base/log.hpp"
#include "net/error.hpp"
#include "net/packet.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <utility>

namespace ba = boost::asio;

namespace net
{


base::Bytes Connection::_read_buffer(base::config::NET_MESSAGE_BUFFER_SIZE);


Connection::Connection(boost::asio::io_context& io_context, boost::asio::ip::tcp::socket&& socket)
    : _io_context{io_context}, _socket{std::move(socket)}
{
    ASSERT(_socket.is_open());
    _network_address =
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


void Connection::close()
{
    ASSERT_SOFT(!_is_closed);
    _is_closed = true;
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


const Endpoint& Connection::getEndpoint() const
{
    return *_network_address;
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
                        LOG_WARNING << "Connection to " << getEndpoint().toString() << " closed";
                        close();
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
                LOG_DEBUG << "Received " << bytes_received << " bytes from " << _network_address->toString();

                if(_is_receiving_enabled) {
                    if(_on_receive) {
                        ReadHandler& user_handler = *_on_receive;
                        user_handler(_read_buffer, bytes_received);
                    }

                    // double-check since the value may be changed - we don't know how long the handler was executing
                    if(_is_receiving_enabled) {
                        receiveOne();
                    }
                }
            }
        });
}


void Connection::setOnReceive(Connection::ReadHandler handler)
{
    _on_receive = std::make_unique<ReadHandler>(std::move(handler));
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
                LOG_DEBUG << "Sent " << bytes_sent << " bytes to " << _network_address->toString();
            }
            _pending_send_messages.pop();

            if(!_pending_send_messages.empty()) {
                sendPendingMessages();
            }
        });
}


void Connection::startSession()
{
    setOnReceive([this](const base::Bytes& message, const std::size_t bytes_received) {
        LOG_DEBUG << "Received: " << message.takePart(0, bytes_received).toString();

        try {
            net::Packet p = net::Packet::deserialize(message.takePart(0, bytes_received));
            LOG_DEBUG << "Received packet type: " << static_cast<int>(p.getType());

            switch(p.getType()) {
                case net::Packet::Type::HANDSHAKE: {
                    LOG_DEBUG << "--- RECEIVED HANDSHAKE ---";
                    break;
                }
                case net::Packet::Type::PING: {
                    LOG_DEBUG << "--- RECEIVED PING ---";
                    net::Packet reply(net::Packet::Type::PONG);
                    send(reply.serialize());
                    break;
                }
                case net::Packet::Type::PONG: {
                    LOG_DEBUG << "--- RECEIVED PONG ---";
                    if(_non_responded_pings) {
                        ASSERT(_on_pong);
                        _non_responded_pings--;
                        PongHandler& ph = *_on_pong;
                        ph();
                    }
                    else {
                        LOG_WARNING << "Received an unexpected PONG from " << getEndpoint().toString();
                    }
                    break;
                }
                case net::Packet::Type::DATA: {

                    break;
                }
                default: {
                    RAISE_ERROR(net::Error, "invalid packet type");
                    break;
                }
            }
        }
        catch(const std::exception& error) {
            LOG_WARNING << std::string{"Received an invalid packet: "} + error.what();
        }
    });

    startReceivingMessages();
}


void Connection::ping()
{
    ASSERT(_on_pong);
    _non_responded_pings++;
    send(net::Packet{net::Packet::Type::PING}.serialize());
}


void Connection::onPong(const PongHandler& on_pong)
{
    _on_pong = std::make_unique<PongHandler>(on_pong);
}


void Connection::onPong(PongHandler&& on_pong)
{
    _on_pong = std::make_unique<PongHandler>(std::move(on_pong));
}


void Connection::onData(const DataHandler& on_data)
{
    _on_data = std::make_unique<DataHandler>(on_data);
}


void Connection::onData(DataHandler&& on_data)
{
    _on_data = std::make_unique<DataHandler>(std::move(on_data));
}

} // namespace net
