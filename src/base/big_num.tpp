#pragma once

#include "big_num.hpp"

namespace base
{
    template<typename T>
    template<typename N>
    BigNum<T>::BigNum(const N& num) : _number(num)
    {}

    template<typename T>
    const std::ostream& BigNum<T>::operator<<(const std::ostream& output) const noexcept
    {
        return output << _number.str;
    }

    template<typename T>
    std::istream& BigNum<T>::operator>>(std::istream& input)
    {
        std::string str;
        input >> str;
        _number = boost::multiprecision::number<T>(str);
    }

    template<typename T>
    BigNum<T> BigNum<T>::operator+(const BigNum<T>& big_num) const
    {
        BigNum<T> res;
        res._number = _number + big_num._number;
        return res;
    }

    template<typename T>
    BigNum<T> BigNum<T>::operator-(const BigNum<T>& big_num) const
    {
        BigNum<T> res;
        res._number = _number - big_num._number;
        return res;
    }

    template<typename T>
    BigNum<T> BigNum<T>::operator*(const BigNum<T>& big_num) const
    {
        BigNum<T> res;
        res._number = _number * big_num._number;
        return res;
    }

    template<typename T>
    BigNum<T> BigNum<T>::operator/(const BigNum<T>& big_num) const
    {
        BigNum<T> res;
        res._number = _number / big_num._number;
        return res;
    }

    template<typename T>
    BigNum<T>& BigNum<T>::operator+=(const BigNum<T>& big_num)
    {
        _number += big_num._number;
        return *this;
    }

    template<typename T>
    BigNum<T>& BigNum<T>::operator-=(const BigNum<T>& big_num)
    {
        _number -= big_num._number;
        return *this;
    }

    template<typename T>
    BigNum<T>& BigNum<T>::operator*=(const BigNum<T>& big_num)
    {
        _number *= big_num._number;
        return *this;
    }

    template<typename T>
    BigNum<T>& BigNum<T>::operator/=(const BigNum<T>& big_num)
    {
        _number /= big_num._number;
        return *this;
    }

    template<typename T>
    bool BigNum<T>::operator!=(const BigNum<T>& big_num) const
    {
        return _number != big_num._number;
    }

    template<typename T>
    bool BigNum<T>::operator==(const BigNum<T>& big_num) const
    {
        return _number == big_num._number;
    }

    template<typename T>
    bool BigNum<T>::operator>(const BigNum<T>& big_num) const
    {
        return _number > big_num._number;
    }

    template<typename T>
    bool BigNum<T>::operator<(const BigNum<T>& big_num) const
    {
        return _number < big_num._number;
    }

    template<typename T>
    bool BigNum<T>::operator>=(const BigNum<T>& big_num) const
    {
        return _number >= big_num._number;
    }

    template<typename T>
    bool BigNum<T>::operator<=(const BigNum<T>& big_num) const
    {
        return _number <= big_num._number;
    }

    template<typename T>
    std::string BigNum<T>::toString() const noexcept
    {
        return _number.str;
    }
}