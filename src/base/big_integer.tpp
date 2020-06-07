#pragma once

#include "big_integer.hpp"

namespace base
{

template<typename T>
template<typename N>
BigInteger<T>::BigInteger(const N& big_int)
  : _number(big_int)
{}


template<typename T>
BigInteger<T> BigInteger<T>::operator+(const BigInteger<T>& other) const
{
    BigInteger<T> res;
    res._number = _number + other._number;
    return res;
}


template<typename T>
BigInteger<T> BigInteger<T>::operator-(const BigInteger<T>& other) const
{
    BigInteger<T> res;
    res._number = _number - other._number;
    return res;
}


template<typename T>
BigInteger<T> BigInteger<T>::operator*(const BigInteger<T>& other) const
{
    BigInteger<T> res;
    res._number = _number * other._number;
    return res;
}


template<typename T>
BigInteger<T> BigInteger<T>::operator/(const BigInteger<T>& other) const
{
    BigInteger<T> res;
    res._number = _number / other._number;
    return res;
}


template<typename T>
BigInteger<T> BigInteger<T>::operator%(const BigInteger<T>& other) const
{
    BigInteger<T> res;
    res._number = _number % other._number;
    return res;
}


template<typename T>
BigInteger<T> BigInteger<T>::operator~() const
{
    return ~_number;
}


template<typename T>
BigInteger<T>& BigInteger<T>::operator+=(const BigInteger<T>& other)
{
    _number += other._number;
    return *this;
}


template<typename T>
BigInteger<T>& BigInteger<T>::operator-=(const BigInteger<T>& other)
{
    _number -= other._number;
    return *this;
}


template<typename T>
BigInteger<T>& BigInteger<T>::operator*=(const BigInteger<T>& other)
{
    _number *= other._number;
    return *this;
}


template<typename T>
BigInteger<T>& BigInteger<T>::operator/=(const BigInteger<T>& other)
{
    _number /= other._number;
    return *this;
}


template<typename T>
bool BigInteger<T>::operator!=(const BigInteger<T>& other) const
{
    return _number != other._number;
}


template<typename T>
bool BigInteger<T>::operator==(const BigInteger<T>& other) const
{
    return _number == other._number;
}


template<typename T>
bool BigInteger<T>::operator>(const BigInteger<T>& other) const
{
    return _number > other._number;
}


template<typename T>
bool BigInteger<T>::operator<(const BigInteger<T>& other) const
{
    return _number < other._number;
}


template<typename T>
bool BigInteger<T>::operator>=(const BigInteger<T>& other) const
{
    return _number >= other._number;
}


template<typename T>
bool BigInteger<T>::operator<=(const BigInteger<T>& other) const
{
    return _number <= other._number;
}


template<typename T>
BigInteger<T>& BigInteger<T>::operator++()
{
    ++_number;
    return *this;
}


template<typename T>
BigInteger<T> BigInteger<T>::operator++(int)
{
    base::BigInteger<T> ret(*this);
    _number++;
    return ret;
}


template<typename T>
BigInteger<T>& BigInteger<T>::operator--()
{
    --_number;
    return *this;
}


template<typename T>
BigInteger<T> BigInteger<T>::operator--(int)
{
    base::BigInteger<T> ret(*this);
    _number--;
    return ret;
}


template<typename T>
std::string BigInteger<T>::toString() const noexcept
{
    return _number.str();
}


template<typename T>
boost::multiprecision::number<T> BigInteger<T>::toMultiNumber() const noexcept
{
    return _number;
}


template<typename T>
void BigInteger<T>::serialize(base::SerializationOArchive& oa) const // TODO: plug
{
    oa.serialize(toString());
}


template<typename T>
BigInteger<T> BigInteger<T>::deserialize(base::SerializationIArchive& ia) // TODO: plug
{
    return BigInteger<T>{ ia.deserialize<std::string>() };
}


template<typename T>
std::ostream& operator<<(std::ostream& output, const BigInteger<T>& big_int)
{
    return output << big_int.toString();
}


template<typename T>
std::istream& operator>>(std::istream& input, BigInteger<T>& big_int)
{
    std::string str;
    input >> str;
    big_int = BigInteger<T>(str);
    return input;
}

} // namespace base