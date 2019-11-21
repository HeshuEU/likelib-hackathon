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


SerializationIArchive& operator>>(SerializationIArchive& ia, base::Bytes& v)
{
    std::size_t size;
    ia >> size;
    v.clear();
    v.reserve(size);
    for(std::size_t i = 0; i < size; ++i) {
        Byte b;
        ia >> b;
        v.append(b);
    }
    return ia;
}


SerializationOArchive& operator<<(SerializationOArchive& ia, const std::string& v)
{
    return ia << base::Bytes(v);
}


SerializationIArchive& operator>>(SerializationIArchive& oa, std::string& v)
{
    base::Bytes b;
    oa >> b;
    v = b.toString();
    return oa;
}


} // namespace base