#pragma once

#include "base/hash.hpp"
#include "base/serialization.hpp"

namespace bc
{

class Address
{
  public:
    Address();

    Address(const char* address);

    // Address(const base::Sha256& hash);

    Address(const std::string& data_string);

    Address(const Address& another) = default;

    Address(Address&& another) = default;

    Address& operator=(const Address& another) = default;

    Address& operator=(Address&& another) = default;

    ~Address() = default;

    std::string toString() const;

    bool operator==(const Address& another) const;

    friend bool operator<(const Address& another_1, const Address& another_2);
    //=================
    base::SerializationOArchive& serialize(base::SerializationOArchive& oa) const;
    static Address deserialize(base::SerializationIArchive& ia);
    //=================
  private:
    base::Bytes _address;
};

bool operator<(const Address& a, const Address& b);

std::ostream& operator<<(std::ostream& os, const Address& address);

extern const Address BASE_ADDRESS;

} // namespace bc