#pragma once

#include "base/hash.hpp"

namespace bc
{

class Address
{
  public:
    Address();

    Address(const char* address);

    Address(const base::Sha256& hash);

    Address(const std::string& data_string);

    Address(const Address& another) = default;

    Address(Address&& another) = default;

    Address& operator=(const Address& another) = default;

    Address& operator=(Address&& another) = default;

    ~Address() = default;

    std::string toString() const;

    bool operator==(const Address& another) const;

    friend bool operator<(const Address& another_1, const Address& another_2);

  private:
    base::Sha256 _hash;
};

bool operator<(const Address& another_1, const Address& another_2);

extern const Address BASE_ADDRESS;

} // namespace bc