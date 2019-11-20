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


SerializationIArchive::SerializationIArchive(const base::Bytes& raw) : _bytes{raw}
{}


SerializationOArchive& operator<<(SerializationOArchive& ia, const base::Bytes& v)
{
    ia << v.size();
    for(Byte b : v.toVector()) {
        ia << b;
    }
    return ia;
}


SerializationIArchive& operator>>(SerializationIArchive& oa, base::Bytes& v)
{
    std::size_t size;
    oa >> size;
    v.clear();
    v.reserve(size);
    for(std::size_t i = 0; i < size; ++i) {
        Byte b;
        oa >> b;
        v.append(b);
    }
    return oa;
}


} // namespace base