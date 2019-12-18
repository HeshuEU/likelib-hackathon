#include "session.hpp"

#include <atomic>

namespace net
{

Session::Session(std::unique_ptr<Connection> connection, std::unique_ptr<Handler> handler)
    : _connection{std::move(connection)}, _handler{std::move(handler)}
{
    ASSERT(_connection);
    setNextId();

    receive(); // starts session
}


Session::~Session()
{
    if(isActive()) {
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
    if(isActive()) {
        _connection->send(data);
    }
}


void Session::send(base::Bytes&& data)
{
    if(isActive()) {
        _connection->send(std::move(data));
    }
}


std::size_t Session::getId() const
{
    return _id;
}


void Session::setNextId()
{
    static std::atomic<std::size_t> next_id;
    _id = next_id++;
}


void Session::close()
{
    if(isActive()) {
        _connection->close();
    }
}


void Session::receive()
{
    static constexpr std::size_t SIZE_OF_MESSAGE_LENGTH_IN_BYTES = 2;
    _connection->receive(SIZE_OF_MESSAGE_LENGTH_IN_BYTES, [this](const base::Bytes& data) {
        auto length = base::fromBytes<std::uint16_t>(data);
        _connection->receive(length, [this](const base::Bytes& data) {
            _handler->onReceive(data);
            if(isActive()) {
                receive();
            }
        });
    });
}


} // namespace net