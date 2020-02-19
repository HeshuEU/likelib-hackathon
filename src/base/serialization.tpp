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


template<typename, typename T>
struct has_deserialize
{
    static_assert(std::integral_constant<T, false>::value, "Second template parameter needs to be of function type.");
};


template<typename C, typename Ret, typename... Args>
struct has_deserialize<C, Ret(Args...)>
{
  private:
    template<typename T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().deserialize(std::declval<Args>()...)), Ret>::type;

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

  public:
    static constexpr bool value = type::value;
};


template<typename T>
class global_deserialize
{
  public:
    T deserialize(base::SerializationIArchive& ia)
    {
        T t;
        ia >> t;
        return t;
    }
};


template<typename T>
class global_deserialize<std::vector<T>>
{
  public:
    std::vector<T> deserialize(base::SerializationIArchive& ia)
    {
        std::vector<T> v;
        std::size_t size = ia.deserialize<std::size_t>();
        for(std::size_t i = 0; i < size; i++) {
            v.push_back(ia.deserialize<T>());
        }
        return v;
    }
};


template<typename T>
class global_deserialize<std::optional<T>>
{
  public:
    std::optional<T> deserialize(base::SerializationIArchive& ia)
    {
        bool do_we_have_a_value;
        ia >> do_we_have_a_value;
        std::optional<T> v;
        if(do_we_have_a_value) {
            T t;
            ia >> t;
            v = t;
        }
        else {
            v = std::nullopt;
        }

        return v;
    }
};


template<>
class global_deserialize<base::Bytes>
{
  public:
    base::Bytes deserialize(base::SerializationIArchive& ia)
    {
        std::size_t size;
        ia >> size;
        base::Bytes bytes(size);
        for(std::size_t i = 0; i < size; ++i) {
            base::Byte b;
            ia >> b;
            bytes[i] = b;
        }
        return bytes;
    }
};


template<>
class global_deserialize<std::string>
{
  public:
    std::string deserialize(base::SerializationIArchive& ia)
    {
        base::Bytes bytes = ia.deserialize<base::Bytes>();
        std::string str = bytes.toString();
        return str;
    }
};


template<typename, typename T>
struct has_serialize
{
    static_assert(std::integral_constant<T, false>::value, "Second template parameter needs to be of function type.");
};


template<typename C, typename Ret, typename... Args>
struct has_serialize<C, Ret(Args...)>
{
  private:
    template<typename T>
    static constexpr auto check(T*) ->
        typename std::is_same<decltype(std::declval<T>().serialize(std::declval<Args>()...)), Ret>::type;

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

  public:
    static constexpr bool value = type::value;
};


template<typename T>
class global_serialize
{
  public:
    void serialize(base::SerializationOArchive& oa, const T& v)
    {
        oa << v;
    }
};


template<typename T>
class global_serialize<std::vector<T>>
{
  public:
    void serialize(base::SerializationOArchive& oa, const std::vector<T>& v)
    {
        oa << v.size();
        for(const auto& x: v) {
            oa.serialize(x);
        }
    }
};


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
T SerializationIArchive::deserialize()
{
    if constexpr(impl::has_deserialize<T, T(base::SerializationIArchive&)>::value) {
        return T::deserialize(*this);
    }
    else {
        return impl::global_deserialize<T>{}.deserialize(*this);
    }
}


template<typename U, typename V>
std::pair<U, V> SerializationIArchive::deserialize()
{
    auto u = deserialize<U>();
    auto v = deserialize<V>();
    return {u, v};
}


template<typename T>
typename std::enable_if<std::is_enum<T>::value, SerializationOArchive&>::type SerializationOArchive::operator<<(
    const T& v)
{
    return *this << static_cast<typename std::underlying_type<T>::type>(v);
}


template<typename T>
void SerializationOArchive::serialize(const T& v)
{
    if constexpr(impl::has_serialize<T, base::SerializationOArchive&(base::SerializationOArchive&)>::value) {
        v.serialize(*this);
    }
    else {
        impl::global_serialize<T>{}.serialize(*this, v);
    }
}


// template<typename T>
// SerializationOArchive& operator<<(SerializationOArchive& oa, const std::vector<T>& v)
// {
//     oa << v.size();
//     for(const auto& x: v) {
//         oa.serialize(x);
//     }
//     return oa;
// }


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
SerializationOArchive& operator<<(SerializationOArchive& oa, const std::pair<U, V>& p)
{
    oa.serialize(p.first);
    oa.serialize(p.second);
    return oa;
}


template<typename T>
base::Bytes toBytes(const T& value)
{
    SerializationOArchive oa;
    oa.serialize(value);
    return std::move(std::move(oa).getBytes());
}


template<typename T>
T fromBytes(const base::Bytes& bytes)
{
    SerializationIArchive ia(bytes);
    T t = ia.deserialize<T>();
    return t;
}


template<typename T, typename TT = typename std::remove_reference<T>::type,
    bool H = std::is_same<decltype(&TT::serialize), decltype(&TT::serialize)>::value>
typename std::enable_if<H, SerializationOArchive&>::type operator<<(SerializationOArchive& oa, T&& t)
{
    std::forward<T>(t).serialize(oa);
    return oa;
}

} // namespace base