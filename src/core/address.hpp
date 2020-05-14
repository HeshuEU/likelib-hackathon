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
    static constexpr std::size_t ADDRESS_BYTES_LENGTH = 20;
    static constexpr std::size_t SIGNATURE_LENGTH = 64;
    //=============================
    explicit Address(const std::string_view& base58_address);
    explicit Address(const base::Bytes& raw);
    explicit Address(const base::FixedBytes<ADDRESS_BYTES_LENGTH>& raw);
    explicit Address(const base::FixedBytes<SIGNATURE_LENGTH>& sign);
    Address(const Address& another) = default;
    Address(Address&& another) = default;
    Address& operator=(const Address& another) = default;
    Address& operator=(Address&& another) = default;
    ~Address() = default;
    //=============================
    [[nodiscard]] std::string toString() const;
    const base::FixedBytes<ADDRESS_BYTES_LENGTH>& getBytes() const noexcept;
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
    base::FixedBytes<ADDRESS_BYTES_LENGTH> _address;
};


std::ostream& operator<<(std::ostream& os, const Address& address);

} // namespace lk
