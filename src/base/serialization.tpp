#pragma once

#include "serialization.hpp"

#include "base/assert.hpp"
#include "base/big_integer.hpp"

#include <boost/asio.hpp>
#include <boost/endian/conversion.hpp>

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
    T deserialize(base::SerializationIArchive& ia, const base::Bytes& _bytes, std::size_t& _index)
    {
        if constexpr (std::is_integral<T>::value) {
            T v;
            static_assert(sizeof(v) == 1 || sizeof(v) == 2 || sizeof(v) == 4 || sizeof(v) == 8,
                          "this integral type is not serializable");

            if (_index + sizeof(T) > _bytes.size()) {
                _index += 120;
            }
            ASSERT(_index + sizeof(T) <= _bytes.size());

            v = *reinterpret_cast<const T*>(_bytes.getData() + _index);
            _index += sizeof(v);
            if constexpr (sizeof(v) != 1) {
                v = base::nativeToBig(v);
            }
            return v;
        }
        else if constexpr (std::is_enum<T>::value) {
            auto v = ia.deserialize<typename std::underlying_type<T>::type>();
            return static_cast<T>(v);
        }
        else {
            static_assert(impl::TrickFalse<T>::value, "type is not deserializable");
        }
    }
};


template<typename T>
class global_deserialize<std::vector<T>>
{
  public:
    std::vector<T> deserialize(base::SerializationIArchive& ia, const base::Bytes&, std::size_t&)
    {
        std::vector<T> v;
        std::size_t size = ia.deserialize<std::size_t>();
        for (std::size_t i = 0; i < size; i++) {
            v.push_back(ia.deserialize<T>());
        }
        return v;
    }
};


template<typename T>
class global_deserialize<std::optional<T>>
{
  public:
    std::optional<T> deserialize(base::SerializationIArchive& ia, const base::Bytes&, std::size_t&)
    {
        auto do_we_have_a_value = ia.deserialize<bool>();
        std::optional<T> v;
        if (do_we_have_a_value) {
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


template<std::size_t S>
class global_deserialize<base::FixedBytes<S>>
{
  public:
    base::FixedBytes<S> deserialize(base::SerializationIArchive& ia, const base::Bytes&, std::size_t&)
    {
        base::FixedBytes<S> fb;
        for (std::size_t i = 0; i < S; ++i) {
            auto b = ia.deserialize<base::Byte>();
            fb[i] = b;
        }
        return fb;
    }
};


template<>
class global_deserialize<base::Bytes>
{
  public:
    base::Bytes deserialize(base::SerializationIArchive& ia, const base::Bytes&, std::size_t&)
    {
        auto size = ia.deserialize<std::size_t>();
        base::Bytes bytes(size);
        for (std::size_t i = 0; i < size; ++i) {
            auto b = ia.deserialize<base::Byte>();
            bytes[i] = b;
        }
        return bytes;
    }
};


template<>
class global_deserialize<std::string>
{
  public:
    std::string deserialize(base::SerializationIArchive& ia, const base::Bytes&, std::size_t&)
    {
        base::Bytes bytes = ia.deserialize<base::Bytes>();
        std::string str = bytes.toString();
        return str;
    }
};


template<typename T>
class global_deserialize<base::BigInteger<T>>
{
  public:
    base::BigInteger<T> deserialize(base::SerializationIArchive& ia, const base::Bytes&, std::size_t&)
    {
        return base::BigInteger<T>{ ia.deserialize<std::string>() };
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
    void serialize(base::SerializationOArchive& oa, const T& v, base::Bytes& _bytes)
    {
        if constexpr (std::is_integral<T>::value) {
            static_assert(sizeof(v) == 1 || sizeof(v) == 2 || sizeof(v) == 4 || sizeof(v) == 8,
                          "this integral type is not serializable");

            if constexpr (sizeof(v) != 1) {
                auto t = base::bigToNative(v);
                _bytes.append(reinterpret_cast<base::Byte*>(&t), sizeof(v));
            }
            else {
                _bytes.append(static_cast<base::Byte>(v));
            }
        }
        else if constexpr (std::is_enum<T>::value) {
            oa.serialize(static_cast<typename std::underlying_type<T>::type>(v));
        }
        else {
            static_assert(impl::TrickFalse<T>::value, "type is not serializable");
        }
    }
};


template<typename T>
class global_serialize<std::vector<T>>
{
  public:
    void serialize(base::SerializationOArchive& oa, const std::vector<T>& v, base::Bytes&)
    {
        oa.serialize(v.size());
        for (const auto& x : v) {
            oa.serialize(x);
        }
    }
};


template<typename T>
class global_serialize<std::optional<T>>
{
  public:
    void serialize(base::SerializationOArchive& oa, const std::optional<T>& v, base::Bytes&)
    {
        if (v) {
            oa.serialize(true);
            oa.serialize(*v);
        }
        else {
            oa.serialize(false);
        }
    }
};


template<std::size_t S>
class global_serialize<base::FixedBytes<S>>
{
  public:
    void serialize(base::SerializationOArchive& oa, const base::FixedBytes<S>& fb, base::Bytes&)
    {
        for (std::size_t i = 0; i < S; i++) {
            oa.serialize(fb[i]);
        }
    }
};


template<>
class global_serialize<base::Bytes>
{
  public:
    void serialize(base::SerializationOArchive& oa, const base::Bytes& bytes, base::Bytes&)
    {
        oa.serialize(bytes.size());
        for (auto b : bytes.toVector()) {
            oa.serialize(b);
        }
    }
};


template<>
class global_serialize<std::string>
{
  public:
    void serialize(base::SerializationOArchive& oa, const std::string& str, base::Bytes&)
    {
        oa.serialize(base::Bytes(str));
    }
};


template<typename T>
class global_serialize<base::BigInteger<T>>
{
  public:
    void serialize(base::SerializationOArchive& oa, const base::BigInteger<T>& n, base::Bytes&)
    {
        oa.serialize(n.str());
    }
};


} // namespace impl


namespace base
{

template<typename T>
T SerializationIArchive::deserialize()
{
    if constexpr (impl::has_deserialize<T, T(base::SerializationIArchive&)>::value) {
        return T::deserialize(*this);
    }
    else {
        return impl::global_deserialize<T>{}.deserialize(*this, _bytes, _index);
    }
}


template<typename U, typename V>
std::pair<U, V> SerializationIArchive::deserialize()
{
    auto u = deserialize<U>();
    auto v = deserialize<V>();
    return { u, v };
}


template<typename T>
void SerializationOArchive::serialize(const T& v)
{
    if constexpr (impl::has_serialize<T, void(base::SerializationOArchive&)>::value) {
        v.serialize(*this);
    }
    else {
        impl::global_serialize<T>{}.serialize(*this, v, _bytes);
    }
}


template<typename U, typename V>
void SerializationOArchive::serialize(const std::pair<U, V>& p)
{
    serialize(p.first);
    serialize(p.second);
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


template<typename T, std::size_t S>
T fromBytes(const base::FixedBytes<S>& bytes)
{
    SerializationIArchive ia(bytes);
    T t = ia.deserialize<T>();
    return t;
}


template<typename T>
T nativeToBig(const T& value) noexcept
{
    return boost::endian::native_to_big(value);
}


template<typename T>
T bigToNative(const T& value) noexcept
{
    return boost::endian::big_to_native(value);
}

} // namespace base