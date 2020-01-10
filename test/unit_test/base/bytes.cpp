#include <boost/test/unit_test.hpp>

#include "base/hash.hpp"
#include "base/bytes.hpp"

#include <memory>

BOOST_AUTO_TEST_CASE(bytes_storage_check)
{
    base::Bytes bytes(128);
    for(std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 3);
    }

    bool all_equal = true;
    for(std::size_t i = 0; i < bytes.size(); ++i) {
        all_equal = all_equal && (static_cast<base::Byte>(i ^ 3) == bytes[i]);
    }

    BOOST_CHECK(all_equal);
}


BOOST_AUTO_TEST_CASE(bytes_constructor_from_array_of_chars)
{
    std::size_t length = 10;
    base::Byte c_str[length];
    for(std::size_t i = 0; i < length; i++){
        c_str[i] = static_cast<base::Byte>(i ^ 3);
    }
    base::Bytes bytes(c_str, length);

    BOOST_CHECK(bytes.size() == length);
    bool res = true;
    for(std::size_t i = 0; i < length; i++){
        res = res && (bytes[i] == c_str[i]);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(bytes_constructor_from_array_of_chars_dynamic)
{
    std::size_t length = 11;
    std::unique_ptr<base::Byte[]> c_str = std::make_unique<base::Byte[]>(length);
    for(std::size_t i = 0; i < length; i++){
        c_str[i] = static_cast<base::Byte>(i ^ 3);
    }
    base::Bytes bytes(c_str.get(), length);

    BOOST_CHECK(bytes.size() == length);
    bool res = true;
    for(std::size_t i = 0; i < length; i++){
        res = res && (bytes[i] == c_str[i]);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(bytes_string_ctor)
{
    base::Bytes b1{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30, 0x02};
    base::Bytes b2("LIKELIB\t2.0\x02");
    BOOST_CHECK(b1 == b2);
}


BOOST_AUTO_TEST_CASE(bytes_size_change)
{
    base::Bytes bytes(128);
    for(std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 33);
    }
    bytes.append(0x15);
    BOOST_CHECK_EQUAL(bytes.size(), 129);
    BOOST_CHECK_EQUAL(bytes[bytes.size() - 1], 0x15);
}


BOOST_AUTO_TEST_CASE(bytes_take_part)
{
    base::Bytes bytes(128);
    for(std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 12);
    }

    base::Bytes answer;
    for(std::size_t i = 20; i < 30; ++i) {
        answer.append(static_cast<base::Byte>(i ^ 12));
    }

    base::Bytes part = bytes.takePart(20, 30);
    BOOST_CHECK(part == answer);
    BOOST_CHECK(!(part != answer));
}

BOOST_AUTO_TEST_CASE(bytes_append)
{
    base::Bytes bytes1(234);
    for(std::size_t i = 0; i < bytes1.size(); ++i) {
        bytes1[i] = static_cast<base::Byte>(i ^ 11);
    }

    base::Bytes bytes2(175);
    for(std::size_t i = 0; i < bytes2.size(); ++i) {
        bytes2[i] = static_cast<base::Byte>(i ^ 13);
    }

    base::Bytes bytes_concat;
    bytes_concat.append(bytes1);
    BOOST_CHECK(bytes_concat.size() == bytes1.size());

    bool res = true;
    for(std::size_t i = 0; i < bytes1.size(); ++i) {
        res = res && (bytes_concat[i] == bytes1[i]);
    }
    BOOST_CHECK(res);

    bytes_concat.append(bytes2);
    BOOST_CHECK(bytes_concat.size() == bytes1.size() + bytes2.size());

    res = true;
    for(std::size_t i = 0; i < bytes2.size(); ++i) {
        res = res && (bytes_concat[bytes1.size() + i] == bytes2[i]);
    }
    BOOST_CHECK(res);
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


BOOST_AUTO_TEST_CASE(bytes_to_hex)
{
    base::Bytes bytes{0x01, 0xFF, 0x02, 0xFE, 0x00};
    BOOST_CHECK_EQUAL(bytes.toHex(), "01ff02fe00");
}


BOOST_AUTO_TEST_CASE(bytes_to_string)
{
    base::Bytes bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30, 0x02};
    BOOST_CHECK_EQUAL(bytes.toString(), "LIKELIB\t2.0\x02");
}


BOOST_AUTO_TEST_CASE(bytes_from_hex)
{
    base::Bytes target_bytes{0x01, 0xFF, 0x02, 0xFE, 0x00};
    auto target_hex = "01ff02fe00";
    auto hex_view = target_bytes.toHex();
    BOOST_CHECK_EQUAL(hex_view, target_hex);

    auto from_hex_bytes = base::Bytes::fromHex(target_hex);
    BOOST_CHECK_EQUAL(from_hex_bytes, target_bytes);
}


BOOST_AUTO_TEST_CASE(bytes_relation_check)
{
    base::Bytes b1 = base::Sha256::compute(base::Bytes("0123")).getBytes();
    base::Bytes b2 = base::Sha256::compute(base::Bytes("123")).getBytes();
    base::Bytes b3 = base::Sha256::compute(base::Bytes("1234")).getBytes();
    base::Bytes b4 = base::Sha256::compute(base::Bytes("1235")).getBytes();
    BOOST_CHECK(b1 < b2);
    BOOST_CHECK(b1 > b3);
    BOOST_CHECK(b1 < b4);
}
