#pragma once

namespace lk
{

template<typename M>
void Peer::sendMessage(const M& msg, net::Connection::SendHandler on_send)
{
    base::SerializationOArchive oa;
    oa.serialize(M::TYPE_ID);
    oa.serialize(msg);
    _session->send(std::move(oa).getBytes(), std::move(on_send));
}

}