#include "connection_acceptor.hpp"

namespace ba = boost::asio;

namespace network
{

ConnectionAcceptor::ConnectionAcceptor(ba::io_context& io_context, const ba::ip::address& bind_ip)
    : _io_context{io_context}, _bind_ip{bind_ip}
{}


void ConnectionAcceptor::startAcceptLoop()
{}


} // namespace network