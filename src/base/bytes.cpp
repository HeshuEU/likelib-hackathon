#include "bytes.hpp"

#include "base/assert.hpp"

#include <iterator>

namespace base
{

Bytes::Bytes()
{}


Bytes::Bytes(std::size_t size) : _raw(size)
{}


Byte& Bytes::operator[](std::size_t index)
{
    ASSERT(index < _raw.size());
    return _raw[index];
}


const Byte& Bytes::operator[](std::size_t index) const
{
    ASSERT(index < _raw.size());
    return _raw[index];
}


Bytes Bytes::takePart(std::size_t begin_index, std::size_t one_past_end_index)
{
    ASSERT(begin_index < one_past_end_index);
    ASSERT(one_past_end_index < _raw.size());

    auto begin = _raw.begin();
    std::advance(begin, begin_index);

    auto end = begin;
    std::advance(end, one_past_end_index - begin_index);

    return Bytes(begin, end);
}


Bytes& Bytes::append(Byte byte)
{
    _raw.push_back(byte);
    return *this;
}


std::size_t Bytes::size() const noexcept
{
    return _raw.size();
}


bool Bytes::operator==(const Bytes& another) const
{
    return _raw == another._raw;
}


bool Bytes::operator!=(const Bytes& another) const
{
    return !(*this == another);
}


const Byte* Bytes::toArray() const
{
    return _raw.data();
}


Byte* Bytes::toArray()
{
    return _raw.data();
}

} // namespace base