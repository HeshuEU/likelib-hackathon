#pragma once

namespace lk
{

// template<typename M>
// void Peer::sendMessage(const M& msg, net::Connection::SendHandler on_send)
//{
//    base::SerializationOArchive oa;
//    oa.serialize(M::TYPE_ID);
//    oa.serialize(msg);
//    _session->send(std::move(oa).getBytes(), std::move(on_send));
//}


template<typename T>
void Peer::endSession(T last_message)
{
    LOG_DEBUG << "ending session";
    try {
        detachFromPools();
        // _requests.send(last_message, [keeper = shared_from_this()] { keeper->_session->close(); });
        _requests.send(last_message);
    }
    catch (const std::exception& e) {
        LOG_WARNING << "Error during peer shutdown: " << e.what();
    }
    catch (...) {
        LOG_WARNING << "Unknown error during peer shutdown";
    }
}


}