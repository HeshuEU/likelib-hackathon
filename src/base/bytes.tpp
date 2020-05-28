#pragma once

#include "bytes.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

#include <openssl/pem.h>

#include <boost/container_hash/hash.hpp>

#include <algorithm>

namespace
{

std::size_t hexToInt(char hex)
{
    if ('0' <= hex && hex <= '9') {
        return hex - '0';
    }
    else if ('a' <= hex && hex <= 'f') {
        return hex - 'a' + 10;
    }
    else if ('A' <= hex && hex <= 'F') {
        return hex - 'A' + 10;
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "Non hex symbol");
    }
}

} // namespace


namespace base
{

template<std::size_t S>
Bytes::Bytes(const FixedBytes<S>& bytes)
  : _raw(bytes.getData(), S)
{}

template<typename I>
Bytes::Bytes(I begin, I end)
  : _raw(begin, end)
{}


template<std::size_t S>
FixedBytes<S>::FixedBytes()
{}


template<std::size_t S>
FixedBytes<S>::FixedBytes(const std::vector<Byte>& bytes)
{
    if (S != bytes.size()) {
        RAISE_ERROR(base::InvalidArgument, "Invalid bytes size for FixedBytes");
    }
    std::copy_n(bytes.begin(), S, _array.begin());
}


template<std::size_t S>
FixedBytes<S>::FixedBytes(const Bytes& bytes)
{
    if (S != bytes.size()) {
        RAISE_ERROR(base::InvalidArgument, "Invalid bytes size for FixedBytes");
    }
    std::copy_n(bytes.toVector().begin(), S, _array.begin());
}


template<std::size_t S>
FixedBytes<S>::FixedBytes(const std::string& str)
{
    if (S != str.size()) {
        RAISE_ERROR(base::InvalidArgument, "Invalid string size for FixedBytes");
    }
    for (std::size_t i = 0; i < S; i++) {
        _array[i] = static_cast<Byte>(str[i]);
    }
}


template<std::size_t S>
FixedBytes<S>::FixedBytes(const Byte* bytes, std::size_t length)
{
    if (S != length) {
        RAISE_ERROR(base::InvalidArgument, "Invalid bytes size for FixedBytes");
    }
    std::copy_n(bytes, S, _array.begin());
}


template<std::size_t S>
FixedBytes<S>::FixedBytes(std::initializer_list<Byte> l)
{
    if (S != l.size()) {
        RAISE_ERROR(base::InvalidArgument, "Invalid initializer list size for FixedBytes");
    }
    for (std::size_t i = 0; i < S; i++) {
        _array[i] = l.begin()[i];
    }
}


template<std::size_t S>
Byte& FixedBytes<S>::operator[](std::size_t index)
{
    ASSERT(index < _array.size());
    return _array[index];
}


template<std::size_t S>
const Byte& FixedBytes<S>::operator[](std::size_t index) const
{
    ASSERT(index < _array.size());
    return _array[index];
}


template<std::size_t S>
std::size_t FixedBytes<S>::size() const noexcept
{
    return S;
}


template<std::size_t S>
const Byte* FixedBytes<S>::getData() const
{
    return _array.data();
}


template<std::size_t S>
Byte* FixedBytes<S>::getData()
{
    return _array.data();
}


template<std::size_t S>
const std::array<Byte, S>& FixedBytes<S>::toArray() const noexcept
{
    return _array;
}


template<std::size_t S>
std::array<Byte, S>& FixedBytes<S>::toArray() noexcept
{
    return _array;
}


template<std::size_t S>
Bytes FixedBytes<S>::toBytes() const
{
    return Bytes(getData(), S);
}


template<std::size_t S>
std::string FixedBytes<S>::toString() const
{
    std::string ret(S, static_cast<char>(0));
    std::size_t index = 0;
    for (const auto& c : _array) {
        ret[index++] = static_cast<char>(c);
    }
    return ret;
}


template<std::size_t S>
bool FixedBytes<S>::operator==(const FixedBytes<S>& another) const
{
    return _array == another._array;
}


template<std::size_t S>
bool FixedBytes<S>::operator!=(const FixedBytes<S>& another) const
{
    return !(*this == another);
}


template<std::size_t S>
bool FixedBytes<S>::operator<(const FixedBytes<S>& another) const
{
    return _array < another._array;
}


template<std::size_t S>
bool FixedBytes<S>::operator>(const FixedBytes<S>& another) const
{
    return _array > another._array;
}


template<std::size_t S>
bool FixedBytes<S>::operator<=(const FixedBytes<S>& another) const
{
    return !(*this > another);
}


template<std::size_t S>
bool FixedBytes<S>::operator>=(const FixedBytes<S>& another) const
{
    return !(*this < another);
}


template<typename T>
std::string toHex(const T& bytes)
{
    static constexpr const char HEX_DIGITS[] = "0123456789abcdef";
    // since every byte is represented by 2 hex digits, we do * 2
    std::string ret(bytes.size() * 2, static_cast<char>(0));
    std::size_t index = 0;
    for (const Byte& c : bytes.toString()) {
        ret[index++] = HEX_DIGITS[c >> 4];
        ret[index++] = HEX_DIGITS[c & 0xF];
    }
    return ret;
}


template<typename T>
T fromHex(const std::string_view& hex_view)
{
    if (hex_view.size() % 2 != 0) {
        RAISE_ERROR(InvalidArgument, "Invalid string length. Odd line length.");
    }

    auto bytes_size = hex_view.size() / 2;
    std::vector<Byte> bytes(bytes_size);
    for (std::size_t current_symbol_index = 0; current_symbol_index < bytes_size; current_symbol_index++) {
        auto index = current_symbol_index * 2;
        auto high_part = hexToInt(hex_view[index]);
        auto low_part = hexToInt(hex_view[index + 1]);
        bytes[current_symbol_index] = (high_part << 4) + low_part;
    }

    return T(bytes);
}


template<typename T>
std::string base64Encode(const T& bytes)
{
    if (bytes.size() == 0) {
        return "";
    }

    BIO* bio_temp = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BUF_MEM* bufferPtr = nullptr;

    std::unique_ptr<BIO, decltype(&::BIO_free_all)> bio(BIO_push(b64, bio_temp), ::BIO_free_all);
    BIO_set_flags(bio.get(), BIO_FLAGS_BASE64_NO_NL);
    if (BIO_write(bio.get(), bytes.getData(), static_cast<int>(bytes.size())) < 1) {
        RAISE_ERROR(CryptoError, "Base64 encode write error");
    }
    if (BIO_flush(bio.get()) < 1) {
        RAISE_ERROR(CryptoError, "Base64 encode flush error");
    }
    if (BIO_get_mem_ptr(bio.get(), &bufferPtr) < 1) {
        if (bufferPtr) {
            BUF_MEM_free(bufferPtr);
        }
        RAISE_ERROR(CryptoError, "Get pointer to memory from base64 error");
    }

    std::string base64_bytes(bufferPtr->data, bufferPtr->length);

    BUF_MEM_free(bufferPtr);
    BIO_set_close(bio.get(), BIO_NOCLOSE);
    return base64_bytes;
}


static constexpr char pszBase58[59] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";


template<typename T>
std::string base58Encode(const T& bytes)
{
    std::size_t current_pos = 0;
    std::size_t zeroes_count = 0;
    std::size_t length = 0;
    while (current_pos != bytes.size() && bytes[current_pos] == 0) {
        current_pos++;
        zeroes_count++;
    }

    base::Bytes b58(bytes.size() * 138 / 100 + 1); // log(256) / log(58)
    while (current_pos != bytes.size()) {
        auto carry = static_cast<std::size_t>(bytes[current_pos]);
        std::size_t i = 0;
        for (auto it = b58.toVector().rbegin(); (carry != 0 || i < length) && (it != b58.toVector().rend());
             it++, i++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }
        length = i;
        current_pos++;
    }
    auto it = b58.toVector().begin() + (b58.size() - length);
    while (it != b58.toVector().end() && *it == 0) {
        it++;
    }

    std::string str;
    str.reserve(zeroes_count + (b58.toVector().end() - it));
    str.assign(zeroes_count, '1');
    while (it != b58.toVector().end()) {
        str += pszBase58[*(it++)];
    }
    return str;
}


} // namespace base


template<std::size_t S>
std::size_t std::hash<base::FixedBytes<S>>::operator()(const base::FixedBytes<S>& k) const
{
    /*
        This hash function is not intended for general use, and isn't guaranteed to be equal
        during separate runs of a program - so please don't use it for any persistent
        storage or communication.
    */
    return boost::hash<decltype(k.toArray())>{}(k.toArray());
}