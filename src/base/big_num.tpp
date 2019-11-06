#pragma once

#include "big_num.hpp"

namespace base
{
    template<typename T>
    template<typename N>
    BigNum<T>::BigNum(const N& num) : _number(num)
    {}

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
        return _number.str();
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& output, const BigNum<T>& big_num)
    {
        return output << big_num.toString();
    }

    template<typename T>
    std::istream& operator>>(std::istream& input, const BigNum<T>& big_num)
    {
        std::string str;
        input >> str;
        big_num = BigNum<T>(str);
        return input;
    }

    template<typename T>
    bool operator==(const BigNum<T>& big_num, const int num)
    {
        return big_num._number == num;
    }
}