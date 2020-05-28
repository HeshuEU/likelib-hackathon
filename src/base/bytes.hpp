#pragma once

#include "base/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

namespace base
{

template<std::size_t S>
class FixedBytes;

class Bytes
{
  public:
    //------------------------
    Bytes() = default;
    explicit Bytes(std::size_t size);
    explicit Bytes(const std::vector<Byte>& bytes);
    explicit Bytes(const std::string& s);
    Bytes(const Byte* bytes, std::size_t length);
    Bytes(std::initializer_list<Byte> l);
    template<std::size_t S>
    explicit Bytes(const FixedBytes<S>& bytes);
    Bytes(const Bytes&) = default;
    Bytes(Bytes&&) = default;
    Bytes& operator=(const Bytes&) = default;
    Bytes& operator=(Bytes&&) = default;
    ~Bytes() = default;
    //------------------------
    template<typename I>
    Bytes(I begin, I end);
    //------------------------
    Byte& operator[](std::size_t index);
    const Byte& operator[](std::size_t index) const;
    //------------------------
    [[nodiscard]] Bytes takePart(std::size_t begin_index, std::size_t one_past_end_index) const;
    //------------------------
    Bytes& append(Byte byte);
    Bytes& append(const Byte* byte, std::size_t length);
    Bytes& append(const Bytes& bytes);
    //------------------------
    std::size_t size() const noexcept;
    //------------------------
    void clear();
    void resize(std::size_t new_size);
    void reserve(std::size_t reserve_size);
    std::size_t capacity() const;
    void shrinkToFit();
    bool isEmpty() const noexcept;
    //------------------------
    const Byte* getData() const;
    Byte* getData();
    //------------------------
    [[nodiscard]] std::vector<Byte>& toVector() noexcept;
    [[nodiscard]] const std::vector<Byte>& toVector() const noexcept;
    //------------------------
    [[nodiscard]] std::string toString() const;
    //------------------------
    bool operator==(const Bytes& another) const;
    bool operator!=(const Bytes& another) const;

    // lexicographical compare
    bool operator<(const Bytes& another) const;
    bool operator>(const Bytes& another) const;
    bool operator<=(const Bytes& another) const;
    bool operator>=(const Bytes& another) const;
    //==============

  private:
    std::vector<Byte> _raw;
};

base::Bytes operator+(const base::Bytes& a, const base::Bytes& b);

std::ostream& operator<<(std::ostream& os, const Bytes& bytes);

template<std::size_t S>
class FixedBytes
{
  public:
    static_assert(S != 0, "FixedBytes length cannot be 0");

    FixedBytes();
    explicit FixedBytes(const std::vector<Byte>& bytes);
    explicit FixedBytes(const Bytes& bytes);
    explicit FixedBytes(const std::string& str);
    FixedBytes(const Byte* bytes, std::size_t length);
    FixedBytes(std::initializer_list<Byte> l);
    FixedBytes(const FixedBytes<S>&) = default;
    FixedBytes(FixedBytes<S>&&) = default;
    FixedBytes& operator=(const FixedBytes<S>&) = default;
    FixedBytes& operator=(FixedBytes<S>&&) = default;
    ~FixedBytes() = default;
    //==============
    Byte& operator[](std::size_t index);
    const Byte& operator[](std::size_t index) const;
    //==============
    std::size_t size() const noexcept;
    //==============
    const Byte* getData() const;
    Byte* getData();
    [[nodiscard]] const std::array<Byte, S>& toArray() const noexcept;
    [[nodiscard]] std::array<Byte, S>& toArray() noexcept;
    Bytes toBytes() const;
    //==============
    [[nodiscard]] std::string toString() const;
    //==============
    bool operator==(const FixedBytes<S>& another) const;
    bool operator!=(const FixedBytes<S>& another) const;

    // lexicographical compare
    bool operator<(const FixedBytes<S>& another) const;
    bool operator>(const FixedBytes<S>& another) const;
    bool operator<=(const FixedBytes<S>& another) const;
    bool operator>=(const FixedBytes<S>& another) const;
    //==============
  private:
    std::array<Byte, S> _array{};
};


template<typename T>
std::string base64Encode(const T& bytes);
base::Bytes base64Decode(std::string_view base64);


template<typename T>
std::string base58Encode(const T& bytes);
base::Bytes base58Decode(std::string_view base58);

template<typename T>
[[nodiscard]] std::string toHex(const T& bytes);

template<typename T>
[[nodiscard]] T fromHex(const std::string_view& hex_view);

} // namespace base

namespace std
{
template<>
struct hash<base::Bytes>
{
    std::size_t operator()(const base::Bytes& k) const;
};

template<std::size_t S>
struct hash<base::FixedBytes<S>>
{
    std::size_t operator()(const base::FixedBytes<S>& k) const;
};

} // namespace std

#include "bytes.tpp"
