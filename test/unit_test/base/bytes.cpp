#include <boost/test/unit_test.hpp>

#include "base/bytes.hpp"
#include "base/hash.hpp"

#include <memory>

BOOST_AUTO_TEST_CASE(bytes_storage_check)
{
    base::Bytes bytes(128);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 3);
    }

    bool all_equal = true;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        all_equal = all_equal && (static_cast<base::Byte>(i ^ 3) == bytes[i]);
    }

    BOOST_CHECK(all_equal);
}


BOOST_AUTO_TEST_CASE(bytes_constructor_from_array_of_chars)
{
    std::size_t length = 10;
    base::Bytes tmp;
    for (std::size_t i = 0; i < length; i++) {
        tmp.append(static_cast<base::Byte>(i ^ 3));
    }
    auto c_str = tmp.getData();
    base::Bytes bytes(c_str, length);

    BOOST_CHECK(bytes.size() == length);
    bool res = true;
    for (std::size_t i = 0; i < length; i++) {
        res = res && (bytes[i] == c_str[i]);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(bytes_constructor_from_array_of_chars_dynamic)
{
    std::size_t length = 11;
    std::unique_ptr<base::Byte[]> c_str = std::make_unique<base::Byte[]>(length);
    for (std::size_t i = 0; i < length; i++) {
        c_str[i] = static_cast<base::Byte>(i ^ 3);
    }
    base::Bytes bytes(c_str.get(), length);

    BOOST_CHECK(bytes.size() == length);
    bool res = true;
    for (std::size_t i = 0; i < length; i++) {
        res = res && (bytes[i] == c_str[i]);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(bytes_string_ctor)
{
    base::Bytes b1{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30, 0x02 };
    base::Bytes b2("LIKELIB\t2.0\x02");
    BOOST_CHECK(b1 == b2);
}


BOOST_AUTO_TEST_CASE(bytes_size_change)
{
    base::Bytes bytes(128);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 33);
    }
    bytes.append(0x15);
    BOOST_CHECK_EQUAL(bytes.size(), 129);
    BOOST_CHECK_EQUAL(bytes[bytes.size() - 1], 0x15);
}


BOOST_AUTO_TEST_CASE(bytes_take_part)
{
    base::Bytes bytes(128);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<base::Byte>(i ^ 12);
    }

    base::Bytes answer;
    for (std::size_t i = 20; i < 30; ++i) {
        answer.append(static_cast<base::Byte>(i ^ 12));
    }

    base::Bytes part = bytes.takePart(20, 30);
    BOOST_CHECK(part == answer);
    BOOST_CHECK(!(part != answer));
}


BOOST_AUTO_TEST_CASE(bytes_take_part_one_symbol)
{
    base::Bytes bytes("eeeaaabbb");
    base::Bytes part = bytes.takePart(7, 8);

    BOOST_CHECK(part.toString() == "b");
}


BOOST_AUTO_TEST_CASE(bytes_append1)
{
    base::Bytes bytes1(234);
    for (std::size_t i = 0; i < bytes1.size(); ++i) {
        bytes1[i] = static_cast<base::Byte>(i ^ 11);
    }

    base::Bytes bytes2(175);
    for (std::size_t i = 0; i < bytes2.size(); ++i) {
        bytes2[i] = static_cast<base::Byte>(i ^ 13);
    }

    base::Bytes bytes_concat;
    bytes_concat.append(bytes1);
    BOOST_CHECK(bytes_concat.size() == bytes1.size());

    bool res = true;
    for (std::size_t i = 0; i < bytes1.size(); ++i) {
        res = res && (bytes_concat[i] == bytes1[i]);
    }
    BOOST_CHECK(res);

    bytes_concat.append(bytes2);
    BOOST_CHECK(bytes_concat.size() == bytes1.size() + bytes2.size());

    res = true;
    for (std::size_t i = 0; i < bytes2.size(); ++i) {
        res = res && (bytes_concat[bytes1.size() + i] == bytes2[i]);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(bytes_append2)
{
    base::Bytes bytes1(333);
    for (std::size_t i = 0; i < bytes1.size(); ++i) {
        bytes1[i] = static_cast<base::Byte>(i ^ 11);
    }

    base::Bytes bytes2(200);
    for (std::size_t i = 0; i < bytes2.size(); i++) {
        bytes2[i] = static_cast<base::Byte>(i ^ 13);
    }

    base::Bytes bytes_concat;
    bytes_concat.append(bytes1);
    std::size_t count_to_concat = bytes2.size() / 2;
    bytes_concat.append(bytes2.getData(), count_to_concat);
    BOOST_CHECK(bytes_concat.size() == bytes1.size() + count_to_concat);

    bool res = true;
    for (std::size_t i = 0; i < bytes1.size(); ++i) {
        res = res && (bytes_concat[i] == bytes1[i]);
    }
    BOOST_CHECK(res);

    res = true;
    for (std::size_t i = 0; i < count_to_concat; ++i) {
        res = res && (bytes_concat[i + bytes1.size()] == bytes2[i]);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(bytes_intializer_list_constructor)
{
    base::Bytes bytes{ 0x1, 0xFF, 0x2, 0xFE };
    BOOST_CHECK_EQUAL(bytes.size(), 4);
    BOOST_CHECK_EQUAL(bytes[0], 0x1);
    BOOST_CHECK_EQUAL(bytes[1], 0xFF);
    BOOST_CHECK_EQUAL(bytes[2], 0x2);
    BOOST_CHECK_EQUAL(bytes[3], 0xFE);
}


BOOST_AUTO_TEST_CASE(bytes_to_hex)
{
    base::Bytes bytes{ 0x01, 0xFF, 0x02, 0xFE, 0x00 };
    BOOST_CHECK_EQUAL(base::toHex<base::Bytes>(bytes), "01ff02fe00");
}


BOOST_AUTO_TEST_CASE(bytes_to_string)
{
    base::Bytes bytes{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30, 0x02 };
    BOOST_CHECK_EQUAL(bytes.toString(), "LIKELIB\t2.0\x02");
}


BOOST_AUTO_TEST_CASE(bytes_from_hex)
{
    base::Bytes target_bytes{ 0x01, 0xFF, 0x02, 0xFE, 0x00 };
    auto target_hex = "01ff02fe00";
    auto hex_view = base::toHex<base::Bytes>(target_bytes);
    BOOST_CHECK_EQUAL(hex_view, target_hex);

    auto from_hex_bytes = base::fromHex<base::Bytes>(target_hex);
    BOOST_CHECK_EQUAL(from_hex_bytes, target_bytes);
}


BOOST_AUTO_TEST_CASE(fixed_bytes_storage_check)
{
    base::FixedBytes<111> fb1;
    for (std::size_t i = 0; i < fb1.size(); ++i) {
        fb1[i] = static_cast<base::Byte>(i ^ 3);
    }

    bool all_equal = true;
    for (std::size_t i = 0; i < fb1.size(); ++i) {
        all_equal = all_equal && (static_cast<base::Byte>(i ^ 3) == fb1[i]);
    }

    BOOST_CHECK(all_equal);
}


BOOST_AUTO_TEST_CASE(fixed_bytes_constructor_from_array_of_chars)
{
    base::FixedBytes<111> fb1;
    for (std::size_t i = 0; i < fb1.size(); i++) {
        fb1[i] = (static_cast<base::Byte>(i ^ 3));
    }
    auto c_str = fb1.getData();
    base::FixedBytes<111> fb2(c_str, fb1.size());

    BOOST_CHECK(fb2.size() == fb1.size());
    bool res = true;
    for (std::size_t i = 0; i < fb1.size(); i++) {
        res = res && (fb2[i] == c_str[i]);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(fixed_bytes_constructor_from_vector)
{
    std::vector<base::Byte> b(111);
    for (std::size_t i = 0; i < b.size(); i++) {
        b[i] = (static_cast<base::Byte>(i ^ 3));
    }

    base::FixedBytes<111> fb(b);

    BOOST_CHECK(fb.size() == b.size());
    bool res = true;
    for (std::size_t i = 0; i < fb.size(); i++) {
        res = res && (b[i] == fb[i]);
    }
    BOOST_CHECK(res);
}


BOOST_AUTO_TEST_CASE(fixed_bytes_string_ctor)
{
    base::FixedBytes<12> b1{ 0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30, 0x02 };
    base::FixedBytes<12> b2("LIKELIB\t2.0\x02");
    BOOST_CHECK(b1 == b2);
    BOOST_CHECK(b1.toString() == b2.toString());
    BOOST_CHECK(b1.toString() == "LIKELIB\t2.0\x02");
}


BOOST_AUTO_TEST_CASE(fixed_bytes_to_hex)
{
    base::FixedBytes<5> bytes{ 0x01, 0xFF, 0x02, 0xFE, 0x00 };
    BOOST_CHECK_EQUAL(base::toHex<base::FixedBytes<5>>(bytes), "01ff02fe00");
}


BOOST_AUTO_TEST_CASE(fixed_bytes_serializatin)
{
    base::FixedBytes<6> fb1{ "Fiasko" };
    base::SerializationOArchive oa;

    oa.serialize(fb1);
    base::SerializationIArchive ia(oa.getBytes());
    auto fb2 = ia.deserialize<base::FixedBytes<6>>();

    BOOST_CHECK(fb1 == fb2);
}


BOOST_AUTO_TEST_CASE(base64_encode_decode)
{
    base::Bytes target_msg("dFM#69356^#-04  @#4-0^\n\n4#0632=-GEJ3dls5s,spi+-5+0");
    auto base64 = base64Encode(target_msg);
    auto decode_base64 = base::base64Decode(base64);

    BOOST_CHECK(base64 == "ZEZNIzY5MzU2XiMtMDQgIEAjNC0wXgoKNCMwNjMyPS1HRUozZGxzNXMsc3BpKy01KzA=");
    BOOST_CHECK(target_msg == decode_base64);
}


BOOST_AUTO_TEST_CASE(base64_encode_fixed_bytes)
{
    base::FixedBytes<50> target_msg("dFM#69356^#-04  @#4-0^\n\n4#0632=-GEJ3dls5s,spi+-5+0");
    auto base64 = base64Encode(target_msg);
    auto decode_base64 = base::base64Decode(base64);

    BOOST_CHECK(base64 == "ZEZNIzY5MzU2XiMtMDQgIEAjNC0wXgoKNCMwNjMyPS1HRUozZGxzNXMsc3BpKy01KzA=");
    BOOST_CHECK(target_msg.toString() == decode_base64.toString());
}


BOOST_AUTO_TEST_CASE(base64_encode_decode_empty)
{
    base::Bytes target_msg("");
    auto base64 = base64Encode(target_msg);
    auto decode_base64 = base::base64Decode(base64);

    BOOST_CHECK(base64 == "");
    BOOST_CHECK(target_msg == decode_base64);
}


BOOST_AUTO_TEST_CASE(base64_encode_decode_one_byte)
{
    base::Bytes target_msg("1");
    auto base64 = base64Encode(target_msg);
    auto decode_base64 = base::base64Decode(base64);

    BOOST_CHECK(base64 == "MQ==");
    BOOST_CHECK(target_msg == decode_base64);
}


BOOST_AUTO_TEST_CASE(base64_encode_fixed_bytes_one_byte)
{
    base::FixedBytes<1> target_msg("1");
    auto base64 = base64Encode(target_msg);
    auto decode_base64 = base::base64Decode(base64);

    BOOST_CHECK(base64 == "MQ==");
    BOOST_CHECK(target_msg.toString() == decode_base64.toString());
}


BOOST_AUTO_TEST_CASE(base58_encode_decode)
{
    base::Bytes target_msg("dFM#69356^#-04  @#4-0^\n\n4#0632=-GEJ3dls5s,spi+-5+0");
    auto base58 = base::base58Encode(target_msg);
    auto decode_base58 = base::base58Decode(base58);
    BOOST_CHECK(base58 == "2EfFY3oougxD9wQVN97fY9zmUuz3dEFnxDzbG8Xga34ArAY8nvx9hhsdtaUYze8CiDABR");
    BOOST_CHECK(decode_base58 == target_msg);
}


BOOST_AUTO_TEST_CASE(base58_encode_fixed_bytes)
{
    base::FixedBytes<50> target_msg("dFM#69356^#-04  @#4-0^\n\n4#0632=-GEJ3dls5s,spi+-5+0");
    auto base58 = base::base58Encode(target_msg);
    auto decode_base58 = base::base58Decode(base58);
    BOOST_CHECK(base58 == "2EfFY3oougxD9wQVN97fY9zmUuz3dEFnxDzbG8Xga34ArAY8nvx9hhsdtaUYze8CiDABR");
    BOOST_CHECK(decode_base58.toString() == target_msg.toString());
}


BOOST_AUTO_TEST_CASE(base58_encode_decode_empty)
{
    base::Bytes target_msg("");
    auto base64 = base58Encode(target_msg);
    auto decode_base64 = base::base58Decode(base64);

    BOOST_CHECK(base64 == "");
    BOOST_CHECK(target_msg == decode_base64);
}
