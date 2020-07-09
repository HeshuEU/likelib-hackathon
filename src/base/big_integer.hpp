#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include <istream>
#include <ostream>

namespace base
{

template<typename T>
using BigInteger = boost::multiprecision::number<T>;

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
