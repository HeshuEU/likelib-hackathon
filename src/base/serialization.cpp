#include "serialization.hpp"

#include <utility>

namespace base
{


void SerializationOArchive::clear()
{
    _bytes.clear();
}


const base::Bytes& SerializationOArchive::getBytes() const& noexcept
{
    return _bytes;
}


base::Bytes&& SerializationOArchive::getBytes() && noexcept
{
    return std::move(_bytes);
}


SerializationIArchive::SerializationIArchive(const base::Bytes& raw)
  : _bytes{ raw }
  , _index{ 0 }
{}


} // namespace base