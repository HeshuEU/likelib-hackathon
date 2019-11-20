#pragma once

#include "base/bytes.hpp"

#include <cstdint>
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
    SerializationIArchive& operator>>(T& v);
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
    SerializationOArchive& operator<<(const T& v);
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


SerializationOArchive& operator<<(SerializationOArchive& ia, const base::Bytes& v);
SerializationIArchive& operator>>(SerializationIArchive& oa, base::Bytes& v);

template<typename T>
SerializationIArchive& operator<<(SerializationIArchive& ia, std::vector<T>& v);

template<typename T>
SerializationOArchive& operator<<(SerializationOArchive& ia, const std::vector<T>& v);


} // namespace base

#include "serialization.tpp"
