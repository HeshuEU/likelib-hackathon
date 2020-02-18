#pragma once

#include "base/crypto.hpp"
#include "base/hash.hpp"
#include "base/serialization.hpp"

namespace bc
{

class Address
{
  public:
    //=============================
    Address(const base::RsaPublicKey& pub);
    Address(const Address& another) = default;
    Address(Address&& another) = default;
    Address& operator=(const Address& another) = default;
    Address& operator=(Address&& another) = default;
    ~Address() = default;
    //=============================
    [[nodiscard]] std::string toString() const;
    //=============================
    bool operator==(const Address& another) const;
    bool operator<(const Address& another) const;
    //=============================
    static Address deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=============================
  private:
    std::string _address;
};



std::ostream& operator<<(std::ostream& os, const Address& address);

extern const Address BASE_ADDRESS;

} // namespace bc