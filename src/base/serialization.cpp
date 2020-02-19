#include "serialization.hpp"

#include "boost/asio.hpp" // includes htons and other

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


SerializationIArchive::SerializationIArchive(const base::Bytes& raw) : _bytes{raw}, _index{0}
{}


SerializationOArchive& operator<<(SerializationOArchive& oa, const base::Bytes& v)
{
    oa << v.size();
    for(Byte b: v.toVector()) {
        oa << b;
    }
    return oa;
}


SerializationOArchive& operator<<(SerializationOArchive& oa, const std::string& v)
{
    return oa << base::Bytes(v);
}


} // namespace base