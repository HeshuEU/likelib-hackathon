#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include <istream>
#include <ostream>

namespace base
{

template<typename T>
class BigInteger
{
  public:
    BigInteger() = default;

    BigInteger(const BigInteger&) = default;

    BigInteger& operator=(const BigInteger&) = default;

    BigInteger(BigInteger&&) = default;

    BigInteger& operator=(BigInteger&&) = default;

    template<typename N>
    BigInteger(const N& num);

    ~BigInteger() = default;

    //----------------------------------

    BigInteger<T> operator+(const BigInteger<T>& big_num) const;

    BigInteger<T> operator-(const BigInteger<T>& big_num) const;

    BigInteger<T> operator*(const BigInteger<T>& big_num) const;

    BigInteger<T> operator/(const BigInteger<T>& big_num) const;

    BigInteger<T>& operator+=(const BigInteger<T>& big_num);

    BigInteger<T>& operator-=(const BigInteger<T>& big_num);

    BigInteger<T>& operator*=(const BigInteger<T>& big_num);

    BigInteger<T>& operator/=(const BigInteger<T>& big_num);

    //----------------------------------

    bool operator!=(const BigInteger<T>& big_num) const;

    bool operator==(const BigInteger<T>& big_num) const;

    bool operator>(const BigInteger<T>& big_num) const;

    bool operator<(const BigInteger<T>& big_num) const;

    bool operator>=(const BigInteger<T>& big_num) const;

    bool operator<=(const BigInteger<T>& big_num) const;

    //----------------------------------

    BigInteger<T>& operator++();

    BigInteger<T>& operator++(int);

    BigInteger<T>& operator--();

    BigInteger<T>& operator--(int);

    //----------------------------------

    std::string toString() const noexcept;

  private:
    boost::multiprecision::number<T> _number;
};

template<typename T>
std::ostream& operator<<(std::ostream& output, const BigInteger<T>& big_num);

template<typename T>
std::istream& operator>>(std::istream& input, BigInteger<T>& big_num);

using Uint256 =
    BigInteger<boost::multiprecision::backends::cpp_int_backend<256, 256,
                                                            boost::multiprecision::cpp_integer_type::unsigned_magnitude,
                                                            boost::multiprecision::cpp_int_check_type::checked, void>>;

using Uint512 =
    BigInteger<boost::multiprecision::backends::cpp_int_backend<512, 512,
                                                            boost::multiprecision::cpp_integer_type::unsigned_magnitude,
                                                            boost::multiprecision::cpp_int_check_type::checked, void>>;
} // namespace base

#include "big_integer.tpp"