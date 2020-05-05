#pragma once

#include "base/bytes.hpp"
#include "base/serialization.hpp"


namespace base
{

class Keccak256
{
  public:
    static constexpr std::size_t KECCAK256_SIZE = 32;
    //----------------------------------
    Keccak256(const Keccak256&) = default;
    Keccak256(Keccak256&&) = default;
    Keccak256(const Bytes& data);
    Keccak256(Bytes&& data);
    Keccak256(const FixedBytes<KECCAK256_SIZE>& data);
    Keccak256(FixedBytes<KECCAK256_SIZE>&& data);
    Keccak256& operator=(const Keccak256&) = default;
    Keccak256& operator=(Keccak256&&) = default;
    ~Keccak256() = default;
    //----------------------------------
    std::string toHex() const;
    const base::FixedBytes<KECCAK256_SIZE>& getBytes() const noexcept;
    //----------------------------------
    static Keccak256 fromHex(const std::string_view& hex_view);
    //----------------------------------
    bool operator==(const Keccak256& another) const;
    bool operator!=(const Keccak256& another) const;
    //----------------------------------
    static Keccak256 compute(const base::Bytes& data);

    template<std::size_t S>
    static Keccak256 compute(const base::FixedBytes<S>& data);
    //----------------------------------
    void serialize(SerializationOArchive& oa) const;
    static Keccak256 deserialize(SerializationIArchive& ia);
    //----------------------------------
  private:
    base::FixedBytes<KECCAK256_SIZE> _bytes;
};

std::ostream& operator<<(std::ostream& os, const Keccak256& sha);

} // namespace base

namespace std
{
template<>
struct hash<base::Keccak256>
{
    std::size_t operator()(const base::Keccak256& k) const;
};
} // namespace std