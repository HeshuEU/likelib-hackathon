#pragma once

#include "base/serialization.hpp"

#include <functional>
#include <iosfwd>

namespace base
{

class Sha256
{
  public:
    static constexpr std::size_t LENGTH = 32; // bytes
    //----------------------------------
    Sha256(const Sha256&) = default;
    explicit Sha256(Sha256&&) = default;
    explicit Sha256(const Bytes& data);
    Sha256(Bytes&& data);
    Sha256(const FixedBytes<LENGTH>& data);
    Sha256(FixedBytes<LENGTH>&& data);
    Sha256& operator=(const Sha256&) = default;
    Sha256& operator=(Sha256&&) = default;
    ~Sha256() = default;
    //----------------------------------
    std::string toHex() const;
    const base::FixedBytes<LENGTH>& getBytes() const noexcept;
    //----------------------------------
    static Sha256 null();
    static Sha256 fromHex(const std::string& hex_view);
    //----------------------------------
    bool operator==(const Sha256& another) const;
    bool operator!=(const Sha256& another) const;
    bool operator<(const Sha256& another) const;
    //----------------------------------
    static Sha256 compute(const base::Bytes& data);

    template<std::size_t S>
    static Sha256 compute(const base::FixedBytes<S>& data);
    //----------------------------------
    void serialize(SerializationOArchive& oa) const;
    static Sha256 deserialize(SerializationIArchive& ia);
    //----------------------------------
  private:
    base::FixedBytes<LENGTH> _bytes;
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
    static constexpr std::size_t LENGTH = 20; // bytes
    //----------------------------------
    Sha1(const Sha1&) = default;
    Sha1(Sha1&&) = default;
    Sha1(const Bytes& data);
    Sha1(Bytes&& data);
    Sha1(const FixedBytes<LENGTH>& data);
    Sha1(FixedBytes<LENGTH>&& data);
    Sha1& operator=(const Sha1&) = default;
    Sha1& operator=(Sha1&&) = default;
    ~Sha1() = default;
    //----------------------------------
    std::string toHex() const;
    const base::FixedBytes<LENGTH>& getBytes() const noexcept;
    //----------------------------------
    static Sha1 fromHex(const std::string_view& hex_view);
    //----------------------------------
    bool operator==(const Sha1& another) const;
    bool operator!=(const Sha1& another) const;
    //----------------------------------
    static Sha1 compute(const base::Bytes& data);

    template<std::size_t S>
    static Sha1 compute(const base::FixedBytes<S>& data);
    //----------------------------------
    void serialize(SerializationOArchive& oa) const;
    static Sha1 deserialize(SerializationIArchive& ia);
    //----------------------------------
  private:
    base::FixedBytes<LENGTH> _bytes;
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


namespace base
{

class Ripemd160
{
  public:
    static constexpr std::size_t LENGTH = 20; // bytes
    //----------------------------------
    Ripemd160(const Ripemd160&) = default;
    Ripemd160(Ripemd160&&) = default;
    Ripemd160(const Bytes& data);
    Ripemd160(Bytes&& data);
    Ripemd160(const FixedBytes<LENGTH>& data);
    Ripemd160(FixedBytes<LENGTH>&& data);
    Ripemd160& operator=(const Ripemd160&) = default;
    Ripemd160& operator=(Ripemd160&&) = default;
    ~Ripemd160() = default;
    //----------------------------------
    std::string toHex() const;
    const base::FixedBytes<LENGTH>& getBytes() const noexcept;
    //----------------------------------
    static Ripemd160 fromHex(const std::string& hex_view);
    //----------------------------------
    bool operator==(const Ripemd160& another) const;
    bool operator!=(const Ripemd160& another) const;
    //----------------------------------
    static Ripemd160 compute(const base::Bytes& data);

    template<std::size_t S>
    static Ripemd160 compute(const base::FixedBytes<S>& data);
    //----------------------------------
    void serialize(SerializationOArchive& oa) const;
    static Ripemd160 deserialize(SerializationIArchive& ia);
    //----------------------------------
  private:
    base::FixedBytes<LENGTH> _bytes;
};

std::ostream& operator<<(std::ostream& os, const Ripemd160& sha);

} // namespace base


namespace std
{

template<>
struct hash<base::Ripemd160>
{
    std::size_t operator()(const base::Ripemd160& k) const;
};

} // namespace std


namespace base
{

class Sha3
{
  public:
    //----------------------------------
    enum class Sha3Type
    {
        Sha3Type224 = 28,
        Sha3Type256 = 32,
        Sha3Type384 = 48,
        Sha3Type512 = 64
    };
    //----------------------------------
    Sha3(const Sha3&) = default;
    Sha3(Sha3&&) = default;
    Sha3(const Bytes& data);
    Sha3(Bytes&& data);
    Sha3& operator=(const Sha3&) = default;
    Sha3& operator=(Sha3&&) = default;
    ~Sha3() = default;
    //----------------------------------
    std::string toHex() const;
    const base::Bytes& getBytes() const noexcept;
    Sha3::Sha3Type getType() const noexcept;
    std::size_t size() const noexcept;
    //----------------------------------
    static Sha3 fromHex(const std::string_view& hex_view);
    //----------------------------------
    bool operator==(const Sha3& another) const;
    bool operator!=(const Sha3& another) const;
    //----------------------------------
    static Sha3 compute(const base::Bytes& data, Sha3Type type);
    static Sha3::Sha3Type getSha3Type(const std::size_t size);
    //----------------------------------
    void serialize(SerializationOArchive& oa) const;
    static Sha3 deserialize(SerializationIArchive& ia);
    //----------------------------------
  private:
    Sha3Type _type;
    base::Bytes _bytes;
};

std::ostream& operator<<(std::ostream& os, const Sha3& sha);

} // namespace base


namespace std
{

template<>
struct hash<base::Sha3>
{
    std::size_t operator()(const base::Sha3& k) const;
};

} // namespace std


namespace base
{

class Keccak256
{
  public:
    static constexpr std::size_t LENGTH = 32; // bytes
    //----------------------------------
    Keccak256(const Keccak256&) = default;
    Keccak256(Keccak256&&) = default;
    Keccak256(const Bytes& data);
    Keccak256(Bytes&& data);
    Keccak256(const FixedBytes<LENGTH>& data);
    Keccak256(FixedBytes<LENGTH>&& data);
    Keccak256& operator=(const Keccak256&) = default;
    Keccak256& operator=(Keccak256&&) = default;
    ~Keccak256() = default;
    //----------------------------------
    std::string toHex() const;
    const base::FixedBytes<LENGTH>& getBytes() const noexcept;
    //----------------------------------
    static Keccak256 fromHex(const std::string_view& hex_view);
    //----------------------------------
    bool operator==(const Keccak256& another) const;
    bool operator!=(const Keccak256& another) const;
    //----------------------------------
    static Keccak256 compute(const base::Bytes& data);
    //----------------------------------
    void serialize(SerializationOArchive& oa) const;
    static Keccak256 deserialize(SerializationIArchive& ia);
    //----------------------------------
  private:
    base::FixedBytes<LENGTH> _bytes;
};

std::ostream& operator<<(std::ostream& os, const Keccak256& sha);

} // namespace base


namespace std
{

template<>
struct hash<base::Keccak256>
{
    std::size_t operator()(const base::Keccak256& k) const;
};

} // namespace std


#include "hash.tpp"