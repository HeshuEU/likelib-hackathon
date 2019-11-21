#include "bytes.hpp"

#include "base/assert.hpp"

#include <iterator>

namespace base
{

Bytes::Bytes()
{}


Bytes::Bytes(std::size_t size) : _raw(size)
{}


Bytes::Bytes(const std::string& s) : _raw(s.length())
{
    std::size_t index = 0;
    for(const char c: s) {
        _raw[index++] = static_cast<Byte>(c);
    }
}


Bytes::Bytes(std::initializer_list<Byte> l) : _raw(l)
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


Bytes Bytes::takePart(std::size_t begin_index, std::size_t one_past_end_index) const
{
    ASSERT(begin_index < one_past_end_index);
    ASSERT(one_past_end_index <= _raw.size());

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


Bytes& Bytes::append(const Bytes& bytes)
{
    _raw.insert(_raw.end(), bytes._raw.begin(), bytes._raw.end());
    return *this;
}


Bytes& Bytes::append(const Byte* byte, std::size_t length)
{
    _raw.reserve(_raw.size() + length);
    for(const Byte* ptr = byte; ptr < byte + length; ++ptr) {
        _raw.push_back(*ptr);
    }
    return *this;
}


void Bytes::clear()
{
    _raw.clear();
}


void Bytes::resize(std::size_t new_size)
{
    _raw.resize(new_size);
}


void Bytes::reserve(std::size_t reserve_size)
{
    _raw.reserve(reserve_size);
}


void Bytes::shrinkToFit()
{
    _raw.shrink_to_fit();
}


std::size_t Bytes::size() const noexcept
{
    return _raw.size();
}


const Byte* Bytes::toArray() const
{
    return _raw.data();
}


Byte* Bytes::toArray()
{
    return _raw.data();
}


std::vector<Byte>& Bytes::toVector() noexcept
{
    return _raw;
}


const std::vector<Byte>& Bytes::toVector() const noexcept
{
    return _raw;
}


std::string Bytes::toHex() const
{
    static constexpr const char HEX_DIGITS[] = "0123456789abcdef";
    // since every byte is represented by 2 hex digits, we do * 2
    std::string ret(_raw.size() * 2, static_cast<char>(0));
    std::size_t index = 0;
    for(const auto& c: _raw) {
        ret[index++] = HEX_DIGITS[c >> 4];
        ret[index++] = HEX_DIGITS[c & 0xF];
    }
    return ret;
}


std::string Bytes::toString() const
{
    std::string ret(_raw.size(), static_cast<char>(0));
    std::size_t index = 0;
    for(const auto& c: _raw) {
        ret[index++] = static_cast<char>(c);
    }
    return ret;
}


bool Bytes::operator==(const Bytes& another) const
{
    return _raw == another._raw;
}


bool Bytes::operator!=(const Bytes& another) const
{
    return !(*this == another);
}


bool Bytes::operator<(const Bytes& another) const
{
    return _raw < another._raw;
}


bool Bytes::operator>(const Bytes& another) const
{
    return _raw > another._raw;
}


bool Bytes::operator<=(const Bytes& another) const
{
    return !(*this > another);
}


bool Bytes::operator>=(const Bytes& another) const
{
    return !(*this < another);
}

} // namespace base
