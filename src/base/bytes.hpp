#pragma once

#include "types.hpp"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

namespace base
{


class Bytes
{
  public:
    //==============
    Bytes();
    explicit Bytes(std::size_t size);
    explicit Bytes(const std::string& s);
    Bytes(std::initializer_list<Byte> l);
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

    Bytes& append(const Bytes& bytes);

    std::size_t size() const noexcept;

    //==============
    const Byte* toArray() const;
    Byte* toArray();

    std::vector<Byte>& toVector() noexcept;
    const std::vector<Byte>& toVector() const noexcept;
    //==============

    std::string toHex() const;

    std::string toString() const;

    //=============
    bool operator==(const Bytes& another) const;
    bool operator!=(const Bytes& another) const;
    //=============
  private:
    std::vector<Byte> _raw;
};

} // namespace base

#include "bytes.tpp"
