#pragma once

#include "serialization.hpp"

#include "base/assert.hpp"

#include <boost/asio.hpp>

#include <functional>


namespace impl
{

template<typename T>
struct TrickFalse : std::false_type
{};


struct Base
{};
struct Derived : Base
{};

template<typename T>
struct IntHolder
{
    using Type = int;
};


template<typename T, typename IntHolder<decltype(T::serialize)>::Type = 0>
void callRightMethod(base::SerializationOArchive& oa, T&& t, Derived)
{
    std::forward<T>(t).serialize(oa);
}


template<typename T>
void callRightMethod(base::SerializationOArchive& oa, T&& t, Base)
{
    oa << std::forward<T>(t);
}


template<typename T>
void chooseSerializationMethod(base::SerializationOArchive& oa, T&& t)
{
    callRightMethod(oa, std::forward<T>(t), Derived{});
}


} // namespace impl


namespace base
{

template<typename T>
typename std::enable_if<std::is_integral<T>::value, SerializationOArchive&>::type SerializationOArchive::operator<<(
    const T& v)
{
    if constexpr(std::is_integral<T>::value) {
        static_assert(sizeof(v) == 1 || sizeof(v) == 2 || sizeof(v) == 4 || sizeof(v) == 8 || sizeof(v) == 16,
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
typename std::enable_if<std::is_integral<T>::value, SerializationIArchive&>::type SerializationIArchive::operator>>(
    T& v)
{
    if constexpr(std::is_integral<T>::value) {
        static_assert(sizeof(v) == 1 || sizeof(v) == 2 || sizeof(v) == 4 || sizeof(v) == 8 || sizeof(v) == 16,
            "this integral type is not serializable");

        ASSERT(_index + sizeof(T) <= _bytes.size());

        v = *reinterpret_cast<const T*>(_bytes.toArray() + _index);
        if constexpr(sizeof(v) == 1) {
            _index++;
        }
        else if constexpr(sizeof(v) == 2) {
            v = ntohs(v);
            _index += 2;
        }
        else if constexpr(sizeof(v) == 4) {
            v = ntohl(v);
            _index += 4;
        }
        else if constexpr(sizeof(v) == 8) {
            std::uint32_t a, b;
            *this >> a >> b;
            v = a;
            v = (v << 32) | b;
        }
        else if constexpr(sizeof(v) == 16) {
            std::uint64_t a, b;
            *this >> a >> b;
            v = a;
            v = (v << 64) | b;
        }
    }
    else {
        static_assert(impl::TrickFalse<T>::value, "type is not deserializable");
    }
    return *this;
}


template<typename T>
typename std::enable_if<std::is_enum<T>::value, SerializationIArchive&>::type SerializationIArchive::operator>>(T& v)
{
    typename std::underlying_type<T>::type value;
    *this >> value;
    v = static_cast<T>(value);
    return *this;
}


template<typename T>
typename std::enable_if<std::is_enum<T>::value, SerializationOArchive&>::type SerializationOArchive::operator<<(
    const T& v)
{
    return *this << static_cast<typename std::underlying_type<T>::type>(v);
}


template<typename T>
typename std::enable_if<!std::is_default_constructible<T>::value, SerializationIArchive&>::type operator>>(
    SerializationIArchive& ia, std::vector<T>& v)
{
    std::size_t size;
    ia >> size;
    v.reserve(size);
    for(std::size_t i = 0; i < size; ++i) {
        v.push_back(std::move(T::deserialize(ia)));
    }
    return ia;
}


template<typename T>
typename std::enable_if<std::is_default_constructible<T>::value, SerializationIArchive&>::type operator>>(
    SerializationIArchive& ia, std::vector<T>& v)
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
SerializationIArchive& operator>>(SerializationIArchive& ia, std::optional<T>& v)
{
    bool do_we_have_a_value;
    ia >> do_we_have_a_value;
    if(do_we_have_a_value) {
        T t;
        ia >> t;
        v = t;
    }
    else {
        v = std::nullopt;
    }

    return ia;
}


template<typename T>
SerializationOArchive& operator<<(SerializationOArchive& oa, const std::optional<T>& v)
{
    if(v) {
        oa << true << *v;
    }
    else {
        oa << false;
    }
    return oa;
}


template<typename U, typename V>
SerializationIArchive& operator>>(SerializationIArchive& ia, std::pair<U, V>& p)
{
    return ia >> p.first >> p.second;
}


template<typename U, typename V>
SerializationOArchive& operator<<(SerializationOArchive& oa, const std::pair<U, V>& p)
{
    return oa << p.first << p.second;
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
    return t;
}


template<typename T, typename TT = typename std::remove_reference<T>::type,
    bool H = std::is_same<decltype(&TT::serialize), decltype(&TT::serialize)>::value>
typename std::enable_if<H, SerializationOArchive&>::type operator<<(SerializationOArchive& oa, T&& t)
{
    std::forward<T>(t).serialize(oa);
    return oa;
}


template<typename T, typename TT = typename std::remove_reference<T>::type,
    bool H = std::is_same<decltype(&TT::deserialize), decltype(&TT::deserialize)>::value>
typename std::enable_if<H, SerializationIArchive&>::type operator>>(SerializationIArchive& ia, T&& t)
{
    t = TT::deserialize(ia);
    return ia;
}

} // namespace base