#include "bytes.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/pem.h>

#include <boost/container_hash/hash.hpp>

namespace base
{

Bytes::Bytes(std::size_t size)
  : _raw(size)
{}


Bytes::Bytes(const std::vector<Byte>& bytes)
  : _raw(bytes)
{}


Bytes::Bytes(const std::string& s)
  : _raw(s.length())
{
    std::size_t index = 0;
    for (const char c : s) {
        _raw[index++] = static_cast<Byte>(c);
    }
}


Bytes::Bytes(const Byte* const bytes, std::size_t length)
  : _raw(bytes, bytes + length)
{}


Bytes::Bytes(std::initializer_list<Byte> l)
  : _raw(l)
{}


Byte& Bytes::operator[](std::size_t index)
{
    ASSERT(index < _raw.size());
    return _raw[index];
}


const Byte& Bytes::operator[](std::size_t index) const
{
    ASSERT(index < _raw.size());
    return _raw[index];
}


Bytes Bytes::takePart(std::size_t begin_index, std::size_t one_past_end_index) const
{
    ASSERT(begin_index < one_past_end_index);
    ASSERT(one_past_end_index <= _raw.size());

    auto begin = _raw.begin();
    std::advance(begin, begin_index);

    auto end = begin;
    std::advance(end, one_past_end_index - begin_index);

    return Bytes(begin, end);
}


Bytes& Bytes::append(Byte byte)
{
    _raw.push_back(byte);
    return *this;
}


Bytes& Bytes::append(const Bytes& bytes)
{
    _raw.insert(_raw.end(), bytes._raw.begin(), bytes._raw.end());
    return *this;
}


Bytes& Bytes::append(const Byte* byte, std::size_t length)
{
    _raw.reserve(_raw.size() + length);
    for (const Byte* ptr = byte; ptr < byte + length; ++ptr) {
        _raw.push_back(*ptr);
    }
    return *this;
}


void Bytes::clear()
{
    _raw.clear();
}


void Bytes::resize(std::size_t new_size)
{
    _raw.resize(new_size);
}


void Bytes::reserve(std::size_t reserve_size)
{
    _raw.reserve(reserve_size);
}


std::size_t Bytes::capacity() const
{
    return _raw.capacity();
}


bool Bytes::isEmpty() const noexcept
{
    return _raw.empty();
}


void Bytes::shrinkToFit()
{
    _raw.shrink_to_fit();
}


std::size_t Bytes::size() const noexcept
{
    return _raw.size();
}


const Byte* Bytes::getData() const
{
    return _raw.data();
}


Byte* Bytes::getData()
{
    return _raw.data();
}


std::vector<Byte>& Bytes::toVector() noexcept
{
    return _raw;
}


const std::vector<Byte>& Bytes::toVector() const noexcept
{
    return _raw;
}


std::string Bytes::toString() const
{
    std::string ret(_raw.size(), static_cast<char>(0));
    std::size_t index = 0;
    for (const auto& c : _raw) {
        ret[index++] = static_cast<char>(c);
    }
    return ret;
}


bool Bytes::operator==(const Bytes& another) const
{
    return _raw == another._raw;
}


bool Bytes::operator!=(const Bytes& another) const
{
    return !(*this == another);
}


bool Bytes::operator<(const Bytes& another) const
{
    return _raw < another._raw;
}


bool Bytes::operator>(const Bytes& another) const
{
    return _raw > another._raw;
}


bool Bytes::operator<=(const Bytes& another) const
{
    return !(*this > another);
}


bool Bytes::operator>=(const Bytes& another) const
{
    return !(*this < another);
}


base::Bytes operator+(const base::Bytes& a, const base::Bytes& b)
{
    base::Bytes ret{ a };
    ret.append(b);
    return ret;
}

std::ostream& operator<<(std::ostream& os, const Bytes& bytes)
{
    return os << toHex<Bytes>(bytes);
}


base::Bytes base64Decode(std::string_view base64)
{
    auto length = base64.length();
    if (length == 0) {
        return base::Bytes();
    }

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new_mem_buf(base64.data(), length);
    base::Bytes ret(length);

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    auto new_length = BIO_read(bio, ret.getData(), length);
    if (new_length < 1) {
        BIO_free_all(bio);
        RAISE_ERROR(CryptoError, "Base64 decode read error");
    }

    BIO_free_all(bio);
    ret.resize(new_length);
    return ret;
}

// clang-format off
static constexpr int8_t mapBase58[256] = {
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
     -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
     22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
     -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
     47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
     -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
 };
// clang-format on


base::Bytes base58Decode(std::string_view base58)
{
    std::size_t current_pos = 0;
    std::size_t zeroes_count = 0;
    std::size_t length = 0;
    while (base58[current_pos] == '1') {
        zeroes_count++;
        current_pos++;
    }

    std::vector<unsigned char> b256((base58.size() - current_pos) * 733 / 1000 + 1); // log(58) / log(256)
    while (current_pos != base58.size()) {
        auto carry = static_cast<int>(mapBase58[static_cast<std::int8_t>(base58[current_pos])]);
        if (carry == -1) {
            RAISE_ERROR(base::InvalidArgument, "Invalid base58 string");
        }
        std::size_t i = 0;
        for (auto it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        length = i;
        current_pos++;
    }

    auto it = b256.begin() + (b256.size() - length);
    base::Bytes ret_bytes(std::vector<base::Byte>(zeroes_count, 0x00));
    while (it != b256.end()) {
        ret_bytes.append(*(it++));
    }
    return ret_bytes;
}
} // namespace base


std::size_t std::hash<base::Bytes>::operator()(const base::Bytes& k) const
{
    return boost::hash_value(k.toVector());
}
