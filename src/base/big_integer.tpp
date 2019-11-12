#pragma once

#include "big_num.hpp"

namespace base
{
template<typename T>
template<typename N>
BigInteger<T>::BigInteger(const N& num) : _number(num)
{}

template<typename T>
BigInteger<T> BigInteger<T>::operator+(const BigInteger<T>& big_num) const
{
    BigInteger<T> res;
    res._number = _number + big_num._number;
    return res;
}

template<typename T>
BigInteger<T> BigInteger<T>::operator-(const BigInteger<T>& big_num) const
{
    BigInteger<T> res;
    res._number = _number - big_num._number;
    return res;
}

template<typename T>
BigInteger<T> BigInteger<T>::operator*(const BigInteger<T>& big_num) const
{
    BigInteger<T> res;
    res._number = _number * big_num._number;
    return res;
}

template<typename T>
BigInteger<T> BigInteger<T>::operator/(const BigInteger<T>& big_num) const
{
    BigInteger<T> res;
    res._number = _number / big_num._number;
    return res;
}

template<typename T>
BigInteger<T>& BigInteger<T>::operator+=(const BigInteger<T>& big_num)
{
    _number += big_num._number;
    return *this;
}

template<typename T>
BigInteger<T>& BigInteger<T>::operator-=(const BigInteger<T>& big_num)
{
    _number -= big_num._number;
    return *this;
}

template<typename T>
BigInteger<T>& BigInteger<T>::operator*=(const BigInteger<T>& big_num)
{
    _number *= big_num._number;
    return *this;
}

template<typename T>
BigInteger<T>& BigInteger<T>::operator/=(const BigInteger<T>& big_num)
{
    _number /= big_num._number;
    return *this;
}

template<typename T>
bool BigInteger<T>::operator!=(const BigInteger<T>& big_num) const
{
    return _number != big_num._number;
}

template<typename T>
bool BigInteger<T>::operator==(const BigInteger<T>& big_num) const
{
    return _number == big_num._number;
}

template<typename T>
bool BigInteger<T>::operator>(const BigInteger<T>& big_num) const
{
    return _number > big_num._number;
}

template<typename T>
bool BigInteger<T>::operator<(const BigInteger<T>& big_num) const
{
    return _number < big_num._number;
}

template<typename T>
bool BigInteger<T>::operator>=(const BigInteger<T>& big_num) const
{
    return _number >= big_num._number;
}

template<typename T>
bool BigInteger<T>::operator<=(const BigInteger<T>& big_num) const
{
    return _number <= big_num._number;
}

template<typename T>
BigInteger<T>& BigInteger<T>::operator++()
{
    ++_number;
    return *this;
}

template<typename T>
BigInteger<T>& BigInteger<T>::operator++(int)
{
    _number++;
    return *this;
}

template<typename T>
BigInteger<T>& BigInteger<T>::operator--()
{
    --_number;
    return *this;
}

template<typename T>
BigInteger<T>& BigInteger<T>::operator--(int)
{
    _number--;
    return *this;
}

template<typename T>
std::string BigInteger<T>::toString() const noexcept
{
    return _number.str();
}

template<typename T>
std::ostream& operator<<(std::ostream& output, const BigInteger<T>& big_num)
{
    return output << big_num.toString();
}

template<typename T>
std::istream& operator>>(std::istream& input, BigInteger<T>& big_num)
{
    std::string str;
    input >> str;
    big_num = BigInteger<T>(str);
    return input;
}

template<typename T>
bool operator==(const BigInteger<T>& big_num, const int num)
{
    return big_num._number == num;
}
} // namespace base