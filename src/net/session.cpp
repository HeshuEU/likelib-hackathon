#include "session.hpp"

namespace net
{

Session::Session(std::unique_ptr<Peer> peer) : _peer{std::move(peer)}
{
    ASSERT(peer);
}


bool Session::isActive() const
{
    return _peer && _peer->isActive();
}


bool Session::isClosed() const
{
    return !isActive();
}


void Session::send(const base::Bytes& data)
{
    if(isActive()) {
        _peer->send(data);
    }
}


void Session::send(base::Bytes&& data)
{
    if(isActive()) {
        _peer->send(std::move(data));
    }
}


Id Session::getId() const
{
    return _peer->getId();
}


void Session::start(MessageHandler handler)
{
    ASSERT(handler);
    if(isActive()) {
        _receive_handler = std::move(handler);
        receive();
    }
}


void Session::stop()
{
    if(isActive()) {
        _peer->close();
    }
}


void Session::receive()
{
    _peer->receive([this](const base::Bytes& data) {
        _receive_handler(*this, data);
        if(isActive()) {
            receive();
        }
    });
}


} // namespace net