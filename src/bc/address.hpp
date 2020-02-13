#pragma once

#include "base/hash.hpp"
#include "base/serialization.hpp"

namespace bc
{

class Address
{
  public:
    //=============================
    Address();
    explicit Address(const char* address);
    explicit Address(base::Sha256 hash);
    explicit Address(const std::string& data_string);
    Address(const Address& another) = default;
    Address(Address&& another) = default;
    Address& operator=(const Address& another) = default;
    Address& operator=(Address&& another) = default;
    ~Address() = default;
    //=============================
    std::string toString() const;
    //=============================
    bool operator==(const Address& another) const;
    bool operator<(const Address& another) const;
    //=============================
  private:
    base::Sha256 _address;
};

base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Address& tx);
base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Address& tx);

std::ostream& operator<<(std::ostream& os, const Address& address);

extern const Address BASE_ADDRESS;

} // namespace bc