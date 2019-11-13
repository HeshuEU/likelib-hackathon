#pragma once

#include <string>

namespace bc
{

class Address
{
  public:
    Address() = default;

    Address(const char* address);

    std::string toString() const;

  private:
    std::string _address;
};

} // namespace bc