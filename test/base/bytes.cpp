#include <boost/test/unit_test.hpp>

#include "base/bytes.hpp"

BOOST_AUTO_TEST_CASE(bytes_storage_check)
{
    base::Bytes bytes(128);
    for(int i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 3);
    }

    bool all_equal = true;
    for(int i = 0; i < bytes.size(); ++i) {
        all_equal = all_equal && (static_cast<base::Byte>(i ^ 3) == bytes[i]);
    }

    BOOST_CHECK(all_equal);
}


BOOST_AUTO_TEST_CASE(bytes_size_change)
{
    base::Bytes bytes(128);
    for(int i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 33);
    }
    bytes.append(0x15);
    BOOST_CHECK_EQUAL(bytes.size(), 129);
    BOOST_CHECK_EQUAL(bytes[bytes.size() - 1], 0x15);
}


BOOST_AUTO_TEST_CASE(bytes_take_part)
{
    base::Bytes bytes(128);
    for(int i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 12);
    }

    base::Bytes answer;
    for(int i = 20; i < 30; ++i) {
        answer.append(static_cast<base::Byte>(i ^ 12));
    }

    base::Bytes part = bytes.takePart(20, 30);
    BOOST_CHECK(part == answer);
    BOOST_CHECK(!(part != answer));
}


BOOST_AUTO_TEST_CASE(bytes_intializer_list_constructor)
{
    base::Bytes bytes{0x1, 0xFF, 0x2, 0xFE};
    BOOST_CHECK_EQUAL(bytes.size(), 4);
    BOOST_CHECK_EQUAL(bytes[0], 0x1);
    BOOST_CHECK_EQUAL(bytes[1], 0xFF);
    BOOST_CHECK_EQUAL(bytes[2], 0x2);
    BOOST_CHECK_EQUAL(bytes[3], 0xFE);
}
