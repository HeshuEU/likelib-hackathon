#pragma once

#include "base/serialization.hpp"

#include <functional>
#include <iosfwd>

namespace base
{

class Sha256
{
  public:
    //----------------------------------
    Sha256(const Sha256&) = default;
    Sha256(Sha256&&) = default;
    Sha256(const Bytes& data);
    Sha256(Bytes&& data);
    Sha256& operator=(const Sha256&) = default;
    Sha256& operator=(Sha256&&) = default;
    ~Sha256() = default;
    //----------------------------------
    std::string toHex() const;
    const base::Bytes& getBytes() const noexcept;
    //----------------------------------
    static Sha256 fromHex(const std::string& hex_view);
    //----------------------------------
    bool operator==(const Sha256& another) const;
    bool operator!=(const Sha256& another) const;
    //----------------------------------
    static Sha256 compute(const base::Bytes& data);
    //----------------------------------
    static SerializationOArchive& serialize(SerializationOArchive& oa, const Sha256& block);
    SerializationOArchive& serialize(SerializationOArchive& oa) const;
    static Sha256 deserialize(SerializationIArchive& ia);
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
    Sha1(const Bytes& another);
    Sha1(Bytes&& another);
    Sha1& operator=(const Sha1&) = default;
    Sha1& operator=(Sha1&&) = default;
    ~Sha1() = default;
    //----------------------------------
    std::string toHex() const;
    const base::Bytes& getBytes() const noexcept;
    //----------------------------------
    static Sha1 fromHex(const std::string& hex_view);
    //----------------------------------
    bool operator==(const Sha1& another) const;
    bool operator!=(const Sha1& another) const;
    //----------------------------------
    static Sha1 compute(const base::Bytes& data);
    //----------------------------------
    static SerializationOArchive& serialize(SerializationOArchive& oa, const Sha1& block);
    SerializationOArchive& serialize(SerializationOArchive& oa) const;
    static Sha1 deserialize(SerializationIArchive& ia);
    //----------------------------------
  private:
    base::Bytes _bytes;
};


std::ostream& operator<<(std::ostream& os, const Sha1& sha);

} // namespace base


namespace std
{
template<>
struct hash<base::Sha1>
{
    std::size_t operator()(const base::Sha1& k) const;
};
} // namespace std
