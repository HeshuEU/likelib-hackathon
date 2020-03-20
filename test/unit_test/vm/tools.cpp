#include <boost/test/unit_test.hpp>

#include "vm/tools.hpp"

BOOST_AUTO_TEST_CASE(tools_uint64_convert)
{
    size_t target_1 = 5;
    auto test_1 = vm::encode(target_1);

    BOOST_CHECK_EQUAL(test_1,
                      base::fromHex<base::Bytes>("0000000000000000000000000000000000000000000000000000000000000005"));

    auto test_2 = vm::decodeAsSizeT(test_1);
    BOOST_CHECK_EQUAL(target_1, test_2);
}


BOOST_AUTO_TEST_CASE(tools_string_convert)
{
    auto target_1 = "one";

    auto test_1 = vm::encode(target_1);
    //    std::cout << test_1.toHex() << std::endl;

    BOOST_CHECK_EQUAL(
      test_1,
      base::fromHex<base::Bytes>(
        "00000000000000000000000000000000000000000000000000000000000000036f6e650000000000000000000000000000000000000000000000000000000000"));

    auto test_2 = vm::decodeAsString(test_1);
    BOOST_CHECK_EQUAL(target_1, test_2);
}
