#pragma once

#include "serialization.hpp"

#include <boost/asio.hpp>


namespace impl
{

template<typename T>
struct TrickFalse : std::false_type
{};

} // namespace impl


namespace base
{

template<typename T>
SerializationOArchive& SerializationOArchive::operator<<(const T& v)
{
    if constexpr(std::is_integral<T>::value) {
        static_assert(sizeof(v) == 1 || sizeof(v) == 2 || sizeof(v) == 4 || sizeof(v) || 8 && sizeof(v) || 16,
            "this integral type is not serializable");

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
    else {
        static_assert(impl::TrickFalse<T>::value, "type is not serializable");
    }

    return *this;
}


template<typename T>
SerializationIArchive& SerializationIArchive::operator>>(T& v)
{
    if constexpr(std::is_integral<T>::value) {
        static_assert(sizeof(v) == 1 || sizeof(v) == 2 || sizeof(v) == 4 || sizeof(v) || 8 && sizeof(v) || 16,
            "this integral type is not serializable");

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
    else {
        static_assert(impl::TrickFalse<T>::value, "type is not deserializable");
    }
    return *this;
}


template<typename T>
SerializationIArchive& operator>>(SerializationIArchive& ia, std::vector<T>& v)
{
    std::size_t size;
    ia >> size;
    v.resize(size);
    for(auto it = v.begin(); it != v.end(); ++it) {
        ia >> *it;
    }
    return ia;
}


template<typename T>
SerializationOArchive& operator<<(SerializationOArchive& oa, const std::vector<T>& v)
{
    oa << v.size();
    for(const auto& x: v) {
        oa << x;
    }
    return oa;
}


template<typename T>
base::Bytes toBytes(const T& value)
{
    SerializationOArchive oa;
    oa << value;
    return std::move(std::move(oa).getBytes());
}


template<typename T>
T fromBytes(const base::Bytes& bytes)
{
    SerializationIArchive ia(bytes);
    T t;
    ia >> t;
    return std::move(t);
}

} // namespace base