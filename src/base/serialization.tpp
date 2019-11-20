#pragma once

#include "serialization.hpp"

#include <boost/asio.hpp>


namespace impl
{

    template<typename T>
    class TrickFalse : std::false_type {
    };

}


namespace base
{

template<typename T>
SerializationIArchive& SerializationIArchive::operator<<(const T& v)
{
    if constexpr(std::is_integral<T>::value) {
        static_assert(sizeof(v) == 1 || sizeof(v) == 2 || sizeof(v) == 4 || sizeof(v) || 8 && sizeof(v) || 16, "this integral type is not serializable");

        if constexpr(sizeof(v) == 1) {
            _bytes.append(static_cast<Byte>(v));
        }
        else if constexpr(sizeof(v) == 2) {
            auto t = htons(v);
            _bytes.append(reinterpret_cast<Byte*>(&t), 2);
        }
        else if constexpr(sizeof(v) == 4) {
            auto t = htonl(v);
            _bytes.append(reinterpret_cast<Byte*>(&t), 4);
        }
        else if constexpr(sizeof(v) == 8) {
            std::uint32_t a = static_cast<std::uint32_t>(v >> 32);
            std::uint32_t b = static_cast<std::uint32_t>(v & 0xFFFFFFFF);
            *this << a << b;
        }
        else if constexpr(sizeof(v) == 16) {
            std::uint64_t b = v & 0xFFFFFFFFFFFFFFFF;
            std::uint64_t a = (v ^ b) >> 64;
            *this << a << b;
        }
    }
    else if constexpr (std::is_same<T, Bytes>::value) {
        *this << v.size();
        _bytes.reserve(_bytes.size() + v.size());
        for(const auto& x : v) {
            _bytes.append(x);
        }
    }
    else {
        static_assert(impl::TrickFalse<T>::value, "type is not serializable");
    }

    return *this;
}


template<typename T>
SerializationOArchive& SerializationOArchive::operator>>(T& v)
{
    if constexpr(std::is_integral<T>::value) {
        static_assert(sizeof(v) == 1 || sizeof(v) == 2 || sizeof(v) == 4 || sizeof(v) || 8 && sizeof(v) || 16, "this integral type is not serializable");

        v = *reinterpret_cast<T*>(_bytes.toArray() + _index);
        if(sizeof(v) == 2) {
            v = ntohs(v);
        }
        else if(sizeof(v) == 4) {
            v = ntohl(v);
        }
        else if(sizeof(v) == 8) {
            std::uint32_t a, b;
            *this >> a >> b;
            v = a;
            v = (v << 32) | b;
        }
        else if(sizeof(v) == 16) {
            std::uint64_t a, b;
            *this >> a >> b;
            v = a;
            v = (v << 64) | b;
        }

        _index += sizeof(v);
    }
    else if constexpr (std::is_same<T, Bytes>::value) {
        std::size_t size;
        *this >> size;
        v.clear();
        v.reserve(size);
        for(std::size_t i = 0; i < size; ++i) {
            v.append(_bytes[_index++]);
        }
    }
    return *this;
}

}