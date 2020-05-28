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
    // TODO: work if some of this types is not defined
    //=================
    void clear();
    //=================
    template<typename T>
    void serialize(const T& v);

    template<typename U, typename V>
    void serialize(const std::pair<U, V>& p);
    //=================
    const base::Bytes& getBytes() const& noexcept;
    base::Bytes&& getBytes() && noexcept;
    //=================

  private:
    base::Bytes _bytes;
};


template<typename T>
base::Bytes toBytes(const T& value);


template<typename T>
T fromBytes(const base::Bytes& bytes);


template<typename T, std::size_t S>
T fromBytes(const base::FixedBytes<S>& bytes);

template<typename T>
T nativeToBig(const T& value) noexcept;


template<typename T>
T bigToNative(const T& value) noexcept;


} // namespace base

#include "serialization.tpp"
