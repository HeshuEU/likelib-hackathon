#pragma once

#include "base/bytes.hpp"

namespace base
{

class Sha256
{
  public:
    //----------------------------------
    Sha256(const Sha256&) = default;
    Sha256(Sha256&&) = default;
    Sha256& operator=(const Sha256&) = default;
    Sha256& operator=(Sha256&&) = default;
    ~Sha256() = default;

    //----------------------------------

    std::string toHex() const;

    std::string toString() const;

    const base::Bytes& getBytes() const noexcept;

    //----------------------------------

    bool operator==(const Sha256& another) const;
    bool operator!=(const Sha256& another) const;

    //----------------------------------

    static Sha256 calcSha256(const base::Bytes& data);

  private:
    Sha256(const Bytes& another);

    base::Bytes _bytes;
};

class Sha1
{
  public:
    //----------------------------------
    Sha1(const Sha1&) = default;
    Sha1(Sha1&&) = default;
    Sha1& operator=(const Sha1&) = default;
    Sha1& operator=(Sha1&&) = default;
    ~Sha1() = default;

    //----------------------------------

    std::string toHex() const;

    std::string toString() const;

    const base::Bytes& getBytes() const noexcept;

    //----------------------------------

    bool operator==(const Sha1& another) const;
    bool operator!=(const Sha1& another) const;

    //----------------------------------

    static Sha1 calcSha1(const base::Bytes& data);

  private:
    Sha1(const Bytes& another);

    base::Bytes _bytes;
};

} // namespace base