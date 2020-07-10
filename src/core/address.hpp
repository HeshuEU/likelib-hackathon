#pragma once

#include "base/crypto.hpp"
#include "base/hash.hpp"
#include "base/serialization.hpp"

namespace lk
{

class Address
{
  public:
    //=============================
    static constexpr std::size_t LENGTH_IN_BYTES = 20;
    static constexpr std::size_t SIGNATURE_LENGTH = 65;
    //=============================
    explicit Address(const std::string_view& base58_address);
    explicit Address(const base::Bytes& raw);
    explicit Address(const base::FixedBytes<LENGTH_IN_BYTES>& raw);
    explicit Address(const base::FixedBytes<SIGNATURE_LENGTH>& sign);
    Address(const Address& another) = default;
    Address(Address&& another) = default;
    Address& operator=(const Address& another) = default;
    Address& operator=(Address&& another) = default;
    ~Address() = default;
    //=============================
    [[nodiscard]] std::string toString() const;
    const base::FixedBytes<LENGTH_IN_BYTES>& getBytes() const noexcept;
    //=============================
    static const Address& null();
    bool isNull() const;
    //=============================
    bool operator==(const Address& another) const;
    bool operator!=(const Address& another) const;
    bool operator<(const Address& another) const;
    //=============================
    static Address deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=============================

  private:
    base::FixedBytes<LENGTH_IN_BYTES> _address;
};


std::ostream& operator<<(std::ostream& os, const Address& address);

} // namespace lk

namespace std
{

template<>
struct hash<lk::Address>
{
    std::size_t operator()(const lk::Address& k) const;
};

} // namespace std