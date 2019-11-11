#include "connection.hpp"

namespace ba = boost::asio;

namespace base::network
{

Connection::Connection(ba::io_context& io_context) : socket{io_context}
{}

} // namespace base::network