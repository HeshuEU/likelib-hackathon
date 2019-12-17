#pragma once

#include "base/bytes.hpp"

#include <functional>
#include <iosfwd>

namespace base
{

class Sha256
{
  public:
    //----------------------------------
    Sha256(const Sha256&) = default;
    Sha256(const std::string& data);
    Sha256(const Bytes& data);
    Sha256(Bytes&& data);
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
    static Sha256 compute(const base::Bytes& data);
    //----------------------------------
  private:
    base::Bytes _bytes;
};

std::ostream& operator<<(std::ostream& os, const Sha256& sha);

} // namespace base

namespace std
{
template<>
struct hash<base::Sha256>
{
    std::size_t operator()(const base::Sha256& k) const;
};
} // namespace std


namespace base
{

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
    static Sha1 compute(const base::Bytes& data);
    //----------------------------------
  private:
    Sha1(const Bytes& another);

    base::Bytes _bytes;
};

} // namespace base


namespace std
{
template<>
struct hash<base::Sha1>
{
    std::size_t operator()(const base::Sha1& k) const;
};
} // namespace std
