#include "protocol.hpp"

#include "base/log.hpp"

namespace lk
{

MessageHandlerRouter::MessageHandlerRouter(bc::Blockchain& blockchain, net::Host& host, net::Peer& peer)
    : _blockchain{blockchain}, _host{host}, _peer{peer}
{}


void MessageHandlerRouter::handle(const base::Bytes& data)
{
    base::SerializationIArchive ia(data);
    MessageType type;
    ia >> type;
    switch(type) {
        case MessageType::TRANSACTION: {
            break;
        }
        case MessageType::BLOCK: {
            break;
        }
        case MessageType::GET_BLOCK: {
            break;
        }
        default: {
            // received an invalid message
            break;
        }
    }
}

} // namespace lk