#pragma once

#include "bytes.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <algorithm>


namespace
{

std::size_t hexToInt(char hex)
{
    if('0' <= hex && hex <= '9') {
        return hex - '0';
    }
    else if('a' <= hex && hex <= 'f') {
        return hex - 'a' + 10;
    }
    else if('A' <= hex && hex <= 'F') {
        return hex - 'A' + 10;
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "Non hex symbol.");
    }
}

} // namespace

namespace base
{

template<typename I>
Bytes::Bytes(I begin, I end) : _raw(begin, end)
{}


template<std::size_t S>
FixedBytes<S>::FixedBytes()
{
    static_assert(S > 0, "FixedBytes length must be longet than 0");
}


template<std::size_t S>
FixedBytes<S>::FixedBytes(const std::vector<Byte>& bytes)
{
    if(S < bytes.size() || bytes.size() == 0) {
        RAISE_ERROR(base::InvalidArgument, "Invalid bytes size for FixedBytes");
    }
    std::copy_n(bytes.begin(), S, _array.begin());
}


template<std::size_t S>
FixedBytes<S>::FixedBytes(const std::string& str)
{
    if(S < str.size() || str.size() == 0) {
        RAISE_ERROR(base::InvalidArgument, "Invalid string size for FixedBytes");
    }
    for(std::size_t i = 0; i < S; i++) {
        _array[i] = static_cast<Byte>(str[i]);
    }
}


template<std::size_t S>
FixedBytes<S>::FixedBytes(const Byte* bytes, std::size_t length)
{
    if(S < length || length == 0) {
        RAISE_ERROR(base::InvalidArgument, "Invalid bytes size for FixedBytes");
    }
    std::copy_n(bytes, S, _array.begin());
}


template<std::size_t S>
FixedBytes<S>::FixedBytes(std::initializer_list<Byte> l)
{
    if(S < l.size() || l.size() == 0) {
        RAISE_ERROR(base::InvalidArgument, "Invalid initializer list size for FixedBytes");
    }
    for(std::size_t i = 0; i < S; i++) {
        _array[i] = l.begin()[i];
    }
}


template<std::size_t S>
Byte& FixedBytes<S>::operator[](std::size_t index)
{
    ASSERT(index < _array.size());
    return _array[index];
}


template<std::size_t S>
const Byte& FixedBytes<S>::operator[](std::size_t index) const
{
    ASSERT(index < _array.size());
    return _array[index];
}


template<std::size_t S>
std::size_t FixedBytes<S>::size() const noexcept
{
    return _array.size();
}


template<std::size_t S>
const Byte* FixedBytes<S>::toArray() const
{
    return _array.data();
}


template<std::size_t S>
Byte* FixedBytes<S>::toArray()
{
    return _array.data();
}


template<std::size_t S>
std::string FixedBytes<S>::toHex() const
{
    return base::toHex(*this);
}


template<std::size_t S>
std::string FixedBytes<S>::toString() const
{
    std::string ret(S, static_cast<char>(0));
    std::size_t index = 0;
    for(const auto& c: _array) {
        ret[index++] = static_cast<char>(c);
    }
    return ret;
}


template<std::size_t S>
bool FixedBytes<S>::operator==(const FixedBytes<S>& another) const
{
    return _array == another._array;
}


template<std::size_t S>
bool FixedBytes<S>::operator!=(const FixedBytes<S>& another) const
{
    return !(*this == another);
}


template<std::size_t S>
bool FixedBytes<S>::operator<(const FixedBytes<S>& another) const
{
    return _array < another._array;
}


template<std::size_t S>
bool FixedBytes<S>::operator>(const FixedBytes<S>& another) const
{
    return _array > another._array;
}


template<std::size_t S>
bool FixedBytes<S>::operator<=(const FixedBytes<S>& another) const
{
    return !(*this > another);
}


template<std::size_t S>
bool FixedBytes<S>::operator>=(const FixedBytes<S>& another) const
{
    return !(*this < another);
}


template<typename T>
std::string toHex(const T& bytes)
{
    static constexpr const char HEX_DIGITS[] = "0123456789abcdef";
    // since every byte is represented by 2 hex digits, we do * 2
    std::string ret(bytes.size() * 2, static_cast<char>(0));
    std::size_t index = 0;
    for(const Byte& c: bytes.toString()) {
        ret[index++] = HEX_DIGITS[c >> 4];
        ret[index++] = HEX_DIGITS[c & 0xF];
    }
    return ret;
}


template<typename T>
T fromHex(const std::string_view& hex_view)
{
    if(hex_view.size() % 2 != 0) {
        RAISE_ERROR(InvalidArgument, "Invalid string length. Odd line length.");
    }

    auto bytes_size = hex_view.size() / 2;
    std::vector<Byte> bytes(bytes_size);
    for(std::size_t current_symbol_index = 0; current_symbol_index < bytes_size; current_symbol_index++) {
        auto index = current_symbol_index * 2;
        auto high_part = hexToInt(hex_view[index]);
        auto low_part = hexToInt(hex_view[index + 1]);
        bytes[current_symbol_index] = (high_part << 4) + low_part;
    }

    return Bytes(bytes);
}
} // namespace base