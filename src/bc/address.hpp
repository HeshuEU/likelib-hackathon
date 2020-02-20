#pragma once

#include "base/hash.hpp"
#include "base/serialization.hpp"

namespace bc
{

class Address
{
  public:
    //=============================
    static Address fromPublicKey(const base::RsaPublicKey& pub);
    Address();
    Address(const std::string_view& base64_address);
    Address(const Address& another) = default;
    Address(Address&& another) = default;
    Address& operator=(const Address& another) = default;
    Address& operator=(Address&& another) = default;
    ~Address() = default;
    //=============================
    [[nodiscard]] std::string toString() const;
    //=============================
    [[nodiscard]] bool isNull() const;
    //=============================
    bool operator==(const Address& another) const;
    bool operator!=(const Address& another) const;
    bool operator<(const Address& another) const;
    //=============================
    static Address deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=============================
  private:
    std::string _address;

    static const std::string& getNullAddressString();
};


std::ostream& operator<<(std::ostream& os, const Address& address);

} // namespace bc
