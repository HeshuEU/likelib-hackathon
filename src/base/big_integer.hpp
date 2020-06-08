#pragma once

#include "base/serialization.hpp"

#include <boost/multiprecision/cpp_int.hpp>

#include <istream>
#include <ostream>

namespace base
{

template<typename T>
class BigInteger
{
  public:
    constexpr BigInteger() = default;

    template<typename N>
    constexpr BigInteger(const N& big_int);

    constexpr BigInteger(const BigInteger&) = default;
    constexpr BigInteger& operator=(const BigInteger&) = default;
    constexpr BigInteger(BigInteger&&) = default;
    constexpr BigInteger& operator=(BigInteger&&) = default;
    ~BigInteger() = default;
    //----------------------------------
    constexpr BigInteger<T> operator+(const BigInteger<T>& other) const;
    constexpr BigInteger<T> operator-(const BigInteger<T>& other) const;
    constexpr BigInteger<T> operator*(const BigInteger<T>& other) const;
    constexpr BigInteger<T> operator/(const BigInteger<T>& other) const;
    constexpr BigInteger<T> operator%(const BigInteger<T>& other) const;
    constexpr BigInteger<T>& operator+=(const BigInteger<T>& other);
    constexpr BigInteger<T>& operator-=(const BigInteger<T>& other);
    constexpr BigInteger<T>& operator*=(const BigInteger<T>& other);
    constexpr BigInteger<T>& operator/=(const BigInteger<T>& other);
    //----------------------------------
    constexpr BigInteger<T> operator~() const;
    //----------------------------------
    constexpr bool operator!=(const BigInteger<T>& other) const;
    constexpr bool operator==(const BigInteger<T>& other) const;
    constexpr bool operator>(const BigInteger<T>& other) const;
    constexpr bool operator<(const BigInteger<T>& other) const;
    constexpr bool operator>=(const BigInteger<T>& other) const;
    constexpr bool operator<=(const BigInteger<T>& other) const;
    //----------------------------------
    constexpr BigInteger<T>& operator++();
    constexpr BigInteger<T> operator++(int);
    constexpr BigInteger<T>& operator--();
    constexpr BigInteger<T> operator--(int);
    //----------------------------------
    std::string toString() const noexcept;
    boost::multiprecision::number<T> toMultiprecisionNumber() const noexcept;
    //----------------------------------
    void serialize(base::SerializationOArchive& oa) const;
    static BigInteger<T> deserialize(base::SerializationIArchive& ia);

  private:
    boost::multiprecision::number<T> _number;
};

template<typename T>
std::ostream& operator<<(std::ostream& output, const BigInteger<T>& big_int);

template<typename T>
std::istream& operator>>(std::istream& input, BigInteger<T>& big_int);

using Uint256 = BigInteger<
  boost::multiprecision::backends::cpp_int_backend<256,
                                                   256,
                                                   boost::multiprecision::cpp_integer_type::unsigned_magnitude,
                                                   boost::multiprecision::cpp_int_check_type::checked,
                                                   void>>;

using Uint512 = BigInteger<
  boost::multiprecision::backends::cpp_int_backend<512,
                                                   512,
                                                   boost::multiprecision::cpp_integer_type::unsigned_magnitude,
                                                   boost::multiprecision::cpp_int_check_type::checked,
                                                   void>>;

} // namespace base

#include "big_integer.tpp"
