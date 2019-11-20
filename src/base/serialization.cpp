#include "serialization.hpp"

#include "boost/asio.hpp" // includes htons and other

#include <utility>


namespace base
{


void SerializationIArchive::clear()
{
    _bytes.clear();
}


const base::Bytes& SerializationIArchive::getBytes() const& noexcept
{
    return _bytes;
}


base::Bytes&& SerializationIArchive::getBytes() && noexcept
{
    return std::move(_bytes);
}


SerializationOArchive::SerializationOArchive(const base::Bytes& raw) : _bytes{raw}
{}


} // namespace base