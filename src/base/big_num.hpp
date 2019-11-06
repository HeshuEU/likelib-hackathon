#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include <ostream>
#include <istream>

namespace base
{
    template<typename T>
    class BigNum
    {
        public:
        //----------------------------------
        BigNum() = default;

        BigNum(const BigNum&) = default;

        BigNum& operator=(const BigNum&) = default;

        BigNum(BigNum&&) = default;

        BigNum& operator=(BigNum&&) = default;

        template<typename N>
        BigNum(const N&);

        ~BigNum() = default;

        //----------------------------------

        BigNum<T> operator+(const BigNum<T>&) const;

        BigNum<T> operator-(const BigNum<T>&) const;

        BigNum<T> operator*(const BigNum<T>&) const;

        BigNum<T> operator/(const BigNum<T>&) const;

        BigNum<T>& operator+=(const BigNum<T>&);

        BigNum<T>& operator-=(const BigNum<T>&);

        BigNum<T>& operator*=(const BigNum<T>&);

        BigNum<T>& operator/=(const BigNum<T>&);

        //----------------------------------

        bool operator!=(const BigNum<T>&) const;

        bool operator==(const BigNum<T>&) const;

        bool operator>(const BigNum<T>&) const;

        bool operator<(const BigNum<T>&) const;

        bool operator>=(const BigNum<T>&) const;

        bool operator<=(const BigNum<T>&) const;

        //----------------------------------

        std::string toString() const noexcept;

        private:

        boost::multiprecision::number<T> _number;
    };

    template<typename T>
    const std::ostream& operator<<(std::ostream&, const BigNum<T>&) noexcept;

    template<typename T>
    std::istream& operator>>(std::istream&, const BigNum<T>&);

    using Uint256 = BigNum<boost::multiprecision::backends::cpp_int_backend<256, 256, 
    boost::multiprecision::cpp_integer_type::unsigned_magnitude, boost::multiprecision::cpp_int_check_type::unchecked, void>>;

    using Uint512 = BigNum<boost::multiprecision::backends::cpp_int_backend<512, 512, 
    boost::multiprecision::cpp_integer_type::unsigned_magnitude, boost::multiprecision::cpp_int_check_type::unchecked, void>>;
}

#include "big_num.tpp"