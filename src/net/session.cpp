#include "session.hpp"

#include "base/assert.hpp"
#include "base/serialization.hpp"

#include <atomic>

namespace
{
static constexpr std::size_t SIZE_OF_MESSAGE_LENGTH_IN_BYTES = 2;
}

namespace net
{

Session::Session(std::unique_ptr<Connection> connection)
  : _connection{ connection.release() }
{
    ASSERT(_connection);
}


Session::~Session()
{
    if (isActive()) {
        close();
    }
}


bool Session::isActive() const
{
    return _connection && !_connection->isClosed();
}


bool Session::isClosed() const
{
    return !isActive();
}


void Session::send(const base::Bytes& data)
{
    if (isActive()) {
        _connection->send(base::toBytes(static_cast<std::uint16_t>(data.size())) + data);
    }
}


void Session::send(base::Bytes&& data)
{
    if (isActive()) {
        _connection->send(base::toBytes(static_cast<std::uint16_t>(data.size())) + data);
    }
}


void Session::send(const base::Bytes& data, Connection::SendHandler on_send)
{
    if (isActive()) {
        _connection->send(base::toBytes(static_cast<std::uint16_t>(data.size())) + data, std::move(on_send));
    }
}


void Session::send(base::Bytes&& data, Connection::SendHandler on_send)
{
    if (isActive()) {
        _connection->send(base::toBytes(static_cast<std::uint16_t>(data.size())) + data, std::move(on_send));
    }
}


void Session::setHandler(std::shared_ptr<Handler> handler)
{
    _handler = std::move(handler);
}


void Session::start()
{
    receive();
}


void Session::close()
{
    if (isActive()) {
        if (_handler) {
            _handler->onClose();
        }
        _connection->close();
    }
}


void Session::reject()
{
    close();
}


void Session::receive()
{
    _connection->receive(SIZE_OF_MESSAGE_LENGTH_IN_BYTES, [this](const base::Bytes& data) {
        _last_seen = base::Time::now();
        auto length = base::fromBytes<std::uint16_t>(data);
        _connection->receive(length, [this](const base::Bytes& data) {
            if (_handler) {
                _handler->onReceive(data);
            }
            if (isActive()) {
                receive();
            }
        });
    });
}


const Endpoint& Session::getEndpoint() const noexcept
{
    return _connection->getEndpoint();
}


const base::Time& Session::getLastSeen() const noexcept
{
    return _last_seen;
}


} // namespace net