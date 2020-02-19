#pragma once

#include "base/bytes.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace base
{

class SerializationIArchive
{
  public:
    //=================
    // it doesn't copy, so the client must be sure that passed bytes are not removed while this class is used
    SerializationIArchive(const Bytes& raw);
    //=================
    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, SerializationIArchive&>::type operator>>(T& v);

    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, SerializationIArchive&>::type operator>>(T& v);
    // TODO: work if some of this types is not defined
    //=================
    template<typename T>
    T deserialize();

    template<typename U, typename V>
    std::pair<U, V> deserialize();
    //=================
  private:
    const base::Bytes& _bytes;
    std::size_t _index;
};


class SerializationOArchive
{
  public:
    //=================
    SerializationOArchive() = default;
    //=================
    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, SerializationOArchive&>::type operator<<(const T& v);

    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, SerializationOArchive&>::type operator<<(const T& v);
    // TODO: work if some of this types is not defined
    //=================
    void clear();
    //=================
    template<typename T>
    void serialize(const T& v);
    //=================
    const base::Bytes& getBytes() const& noexcept;
    base::Bytes&& getBytes() && noexcept;
    //=================

  private:
    base::Bytes _bytes;
};


SerializationOArchive& operator<<(SerializationOArchive& oa, const base::Bytes& v);


SerializationOArchive& operator<<(SerializationOArchive& oa, const std::string& v);

template<typename T>
SerializationOArchive& operator<<(SerializationOArchive& oa, const std::vector<T>& v);


template<typename T>
SerializationOArchive& operator<<(SerializationOArchive& oa, const std::optional<T>& v);


template<typename U, typename V>
SerializationOArchive& operator<<(SerializationOArchive& oa, const std::pair<U, V>& p);


template<typename T, typename TT = typename std::remove_reference<T>::type,
    bool H = std::is_same<decltype(&TT::serialize), decltype(&TT::serialize)>::value>
typename std::enable_if<H, SerializationOArchive&>::type operator<<(SerializationOArchive& oa, T&& t);


// template<typename T, typename TT = typename std::remove_reference<T>::type,
//     bool H = std::is_same<decltype(&TT::deserialize), decltype(&TT::deserialize)>::value>
// typename std::enable_if<H, SerializationIArchive&>::type operator>>(SerializationIArchive& ia, T&& t);


template<typename T>
base::Bytes toBytes(const T& value);


template<typename T>
T fromBytes(const base::Bytes& bytes);


} // namespace base

#include "serialization.tpp"
