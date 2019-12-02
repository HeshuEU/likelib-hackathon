#pragma once

#include "base/bytes.hpp"

#include <cstdint>
#include <type_traits>
#include <string>
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
    // TODO: work if some of this types is not defined
    //=================
  private:
    base::Bytes _bytes;
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
    // TODO: work if some of this types is not defined
    //=================
    void clear();
    //=================
    const base::Bytes& getBytes() const& noexcept;
    base::Bytes&& getBytes() && noexcept;
    //=================

  private:
    base::Bytes _bytes;
};


SerializationOArchive& operator<<(SerializationOArchive& ia, const base::Bytes& v);
SerializationIArchive& operator>>(SerializationIArchive& oa, base::Bytes& v);


SerializationOArchive& operator<<(SerializationOArchive& ia, const std::string& v);
SerializationIArchive& operator>>(SerializationIArchive& oa, std::string& v);


template<typename T>
SerializationIArchive& operator<<(SerializationIArchive& ia, std::vector<T>& v);

template<typename T>
SerializationOArchive& operator<<(SerializationOArchive& ia, const std::vector<T>& v);


template<typename T>
base::Bytes toBytes(const T& value);


template<typename T>
T fromBytes(const base::Bytes& bytes);


} // namespace base

#include "serialization.tpp"