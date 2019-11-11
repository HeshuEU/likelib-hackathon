#pragma once

#include "types.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace base
{


class Bytes
{
  public:
    //==============
    Bytes();
    explicit Bytes(std::size_t size);
    Bytes(const Bytes&) = default;
    Bytes(Bytes&&) = default;
    Bytes& operator=(const Bytes&) = default;
    Bytes& operator=(Bytes&&) = default;
    ~Bytes() = default;
    //==============

    template<typename I>
    Bytes(I begin, I end);

    Byte& operator[](std::size_t index);
    const Byte& operator[](std::size_t index) const;

    Bytes takePart(std::size_t begin_index, std::size_t one_past_end_index);

    Bytes& append(Byte byte);

    std::size_t size() const noexcept;

    //=============
    bool operator==(const Bytes& another) const;
    bool operator!=(const Bytes& another) const;
    //=============
  private:
    std::vector<Byte> _raw;
};

} // namespace base

#include "bytes.tpp"
