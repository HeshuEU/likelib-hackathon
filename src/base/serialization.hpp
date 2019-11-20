#pragma once

#include "base/bytes.hpp"

#include <cstdint>
#include <type_traits>

namespace base
{

class SerializationIArchive
{
  public:
    //=================
    SerializationIArchive() = default;
    //=================
    template<typename T>
    SerializationIArchive& operator<<(const T& v);
    // TODO: work if some of this types is not defined
    //=================
    void clear();
    //=================
    const base::Bytes& getBytes() const & noexcept;
    base::Bytes&& getBytes() && noexcept;
    //=================

  private:
    base::Bytes _bytes;
};


class SerializationOArchive
{
public:
    //=================
    // it doesn't copy, so the client must be sure that passed bytes are not removed while this class is used
    SerializationOArchive(const base::Bytes& raw);
    //=================
    template<typename T>
    SerializationOArchive& operator>>(T& v);
    // TODO: work if some of this types is not defined
    //=================
private:
    base::Bytes _bytes;
    std::size_t _index;
};

} // namespace base

#include "serialization.tpp"
