#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include <istream>
#include <ostream>

namespace base
{

template<typename T>
class BigNum
{
  public:
    BigNum() = default;

    BigNum(const BigNum&) = default;

    BigNum& operator=(const BigNum&) = default;

    BigNum(BigNum&&) = default;

    BigNum& operator=(BigNum&&) = default;

    template<typename N>
    BigNum(const N& num);

    ~BigNum() = default;

    //----------------------------------

    BigNum<T> operator+(const BigNum<T>& big_num) const;

    BigNum<T> operator-(const BigNum<T>& big_num) const;

    BigNum<T> operator*(const BigNum<T>& big_num) const;

    BigNum<T> operator/(const BigNum<T>& big_num) const;

    BigNum<T>& operator+=(const BigNum<T>& big_num);

    BigNum<T>& operator-=(const BigNum<T>& big_num);

    BigNum<T>& operator*=(const BigNum<T>& big_num);

    BigNum<T>& operator/=(const BigNum<T>& big_num);

    //----------------------------------

    bool operator!=(const BigNum<T>& big_num) const;

    bool operator==(const BigNum<T>& big_num) const;

    bool operator>(const BigNum<T>& big_num) const;

    bool operator<(const BigNum<T>& big_num) const;

    bool operator>=(const BigNum<T>& big_num) const;

    bool operator<=(const BigNum<T>& big_num) const;

    //----------------------------------

    BigNum<T>& operator++();

    BigNum<T>& operator++(int);

    BigNum<T>& operator--();

    BigNum<T>& operator--(int);

    //----------------------------------

    std::string toString() const noexcept;

  private:
    boost::multiprecision::number<T> _number;
};

template<typename T>
std::ostream& operator<<(std::ostream& output, const BigNum<T>& big_num);

template<typename T>
std::istream& operator>>(std::istream& input, BigNum<T>& big_num);

using Uint256 =
    BigNum<boost::multiprecision::backends::cpp_int_backend<256, 256,
                                                            boost::multiprecision::cpp_integer_type::unsigned_magnitude,
                                                            boost::multiprecision::cpp_int_check_type::checked, void>>;

using Uint512 =
    BigNum<boost::multiprecision::backends::cpp_int_backend<512, 512,
                                                            boost::multiprecision::cpp_integer_type::unsigned_magnitude,
                                                            boost::multiprecision::cpp_int_check_type::checked, void>>;
} // namespace base

#include "big_num.tpp"