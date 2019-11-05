#include <boost/test/unit_test.hpp>

#include "base/big_num.hpp"

#include <string>

BOOST_AUTO_TEST_CASE(BigNum_constructor)
{
    base::Uint256 num1(3u);
    BOOST_CHECK(num1 == base::Uint256(3u));

    base::Uint256 num2(765ll);
    BOOST_CHECK(num2 == base::Uint256(765ll));

    base::Uint256 num3("654321");
    BOOST_CHECK(num3 == base::Uint256("654321"));

    bool res = true; 
    for(uint32_t i = 0; i < 1000; i++){
        base::Uint256 num(i);
        res = res && (num == base::Uint256(i));
    }
    BOOST_CHECK(res);

    res = true; 
    for(uint32_t i = 0; i < 1000; i++){
        base::Uint256 num(std::to_string(i));
        res = res && (num == base::Uint256(i));
    }
    BOOST_CHECK(res);

    std::string str_num;
    for(uint32_t i = 0; i < 256; i++){
           str_num += '1';
    }
    base::Uint256 num256(str_num);
    BOOST_CHECK(num256 == base::Uint256(str_num));

    base::Uint512 num4(987u);
    BOOST_CHECK(num4 == base::Uint512(987u));

    base::Uint512 num5(111ll);
    BOOST_CHECK(num5 == base::Uint512(111ll));

    base::Uint512 num6("123456");
    BOOST_CHECK(num6 == base::Uint512("123456"));

    res = true;
    for(uint32_t i = 1000; i < 2000; i++){
        base::Uint512 num(i);
        res = res && (num == base::Uint512(i));
    }
    BOOST_CHECK(res);
    
    res = true;
    for(uint32_t i = 1000; i < 2000; i++){
        base::Uint512 num(std::to_string(i));
        res = res && (num == base::Uint512(i));
    }
    BOOST_CHECK(res);

    str_num = "";
    for(uint32_t i = 0; i < 512; i++){
           str_num += '1';
    }
    base::Uint512 num512(str_num);
    BOOST_CHECK(num512 == base::Uint512(str_num));
}


BOOST_AUTO_TEST_CASE(BigNum_constructor_copy_move)
{
    base::Uint256 num1(456u);
    base::Uint256 num2 = num1;
    BOOST_CHECK(num1 == num2);

    base::Uint256 num3(789ll);
    base::Uint256 num4 = num3;
    
}
