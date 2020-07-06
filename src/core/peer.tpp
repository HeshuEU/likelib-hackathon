#pragma once

#include <boost/stacktrace/stacktrace.hpp>

namespace lk
{

template<typename T>
void Peer::endSession(T last_message)
{
    try {
        detachFromPools();
        _requests.send(last_message, [keeper = shared_from_this()] { keeper->_session->close(); });
    }
    catch (const std::exception& e) {
        LOG_WARNING << "Error during peer shutdown: " << e.what();
    }
    catch (...) {
        LOG_WARNING << "Unknown error during peer shutdown";
    }
}


template<typename T>
base::Bytes Requests::prepareMessage(const T& msg)
{
    base::SerializationOArchive oa;
    oa.serialize(_next_message_id++);
    oa.serialize(T::TYPE_ID);
    oa.serialize(msg);
    return std::move(oa).getBytes();
}


template<typename T>
void Requests::send(const T& msg, net::Connection::SendHandler cb)
{
    if (auto s = _session.lock()) {
        s->send(prepareMessage(msg), std::move(cb));
    }
    else {
        RAISE_ERROR(net::SendOnClosedConnection, "attempt to request on closed connection");
    }
}


template<typename T>
void Requests::requestWaitResponseById(const T& msg,
                                       Request::ResponseCallback response_callback,
                                       Request::TimeoutCallback timeout_callback,
                                       net::Connection::SendHandler cb)
{
    if (auto s = _session.lock()) {
        _active_requests.own(std::make_shared<Request>(_active_requests,
                                                       _next_message_id++,
                                                       std::move(response_callback),
                                                       std::move(timeout_callback),
                                                       _io_context));
        s->send(prepareMessage(msg), std::move(cb));
    }
    else {
        RAISE_ERROR(net::SendOnClosedConnection, "attempt to request on closed connection");
    }
}


}