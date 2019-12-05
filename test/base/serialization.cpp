#include <boost/test/unit_test.hpp>

#include "base/serialization.hpp"

BOOST_AUTO_TEST_CASE(serialization_sanity_check1)
{
    base::SerializationOArchive oa;
    oa << base::Byte{0x12} << -35 << -7ll;

    base::SerializationIArchive ia(oa.getBytes());
    base::Byte a;
    int b;
    long long c;
    ia >> a >> b >> c;
    BOOST_CHECK_EQUAL(a, 0x12);
    BOOST_CHECK_EQUAL(b, -35);
    BOOST_CHECK_EQUAL(c, -7);
}


BOOST_AUTO_TEST_CASE(serialization_sanity_check2)
{
    const std::size_t aa = 0x35;
    base::Bytes bb{0x1, 0x3, 0x5, 0x7, 0x15};
    const std::vector<std::string> cc{"a", "b", "c"};

    base::SerializationOArchive oa;
    oa << aa << bb << cc;

    base::SerializationIArchive ia(oa.getBytes());

    std::size_t a;
    ia >> a;
    BOOST_CHECK_EQUAL(a, aa);

    base::Bytes b;
    ia >> b;
    BOOST_CHECK(b == bb);

    std::vector<std::string> c;
    ia >> c;
    BOOST_CHECK(c == cc);
}


BOOST_AUTO_TEST_CASE(serialization_operators_input_output_Bytes)
{
    base::SerializationOArchive oa;
    std::vector<char> v1{'f', '!', '*', 'a'};
    std::vector<int> v2{2200, -8001, 111, 77, -99976};
    std::vector<long long> v3{20000000000};
    std::vector<unsigned char> v4;
    oa << v1 << v2 << v3 << v4;

    base::SerializationIArchive ia(oa.getBytes());
    std::vector<char> v11;
    std::vector<int> v22;
    std::vector<long long> v33;
    std::vector<unsigned char> v44;
    ia >> v11 >> v22 >> v33 >> v44;
    BOOST_CHECK(v1 == v11);
    BOOST_CHECK(v2 == v22);
    BOOST_CHECK(v3 == v33);
    BOOST_CHECK(v4 == v44);
}


BOOST_AUTO_TEST_CASE(serialization_operators_input_output_string)
{
    base::SerializationOArchive oa;
    std::string str1 = "\n dfg345 Talant\n >12";
    std::string str2 = " _Krek +-* \n\n \\ \\\\";
    std::string str3 = "  !*_ sdf * 345 vbm  ?  \n";
    std::string str4;
    oa << str1 << str2 << str3 << str4 << "refhu\n / sdfg// w04541(!&$ \\";

    base::SerializationIArchive ia(oa.getBytes());
    std::string s1, s2, s3, s4, s5;
    ia >> s1 >> s2 >> s3 >> s4 >> s5;
    BOOST_CHECK(str1 == s1);
    BOOST_CHECK(str2 == s2);
    BOOST_CHECK(str3 == s3);
    BOOST_CHECK(str4 == s4);
    BOOST_CHECK(s5 == "refhu\n / sdfg// w04541(!&$ \\");
}


BOOST_AUTO_TEST_CASE(serialization_operators_input_output_combo)
{
    base::SerializationOArchive oa;
    std::string str1 = "\n sdfwg23793 238 JS sdf! //";
    int i1 = 345;
    base::Bytes bytes1{"359 sdf 54986 \n // ]\\ sdfe"};
    long long ll1 = 90000000000;
    std::vector<unsigned long long> arr1{29385710293, 9234825982, 2348, 92312039, 345986};
    oa << str1 << i1 << bytes1 << ll1 << arr1 << "Combined test \n / sdf @$()29 ";

    base::SerializationIArchive ia(oa.getBytes());
    std::string str2;
    int i2;
    base::Bytes bytes2;
    long long ll2;
    std::vector<unsigned long long> arr2;
    std::string str_t;
    ia >> str2 >> i2 >> bytes2 >> ll2 >> arr2 >> str_t;
    BOOST_CHECK(str1 == str2);
    BOOST_CHECK(i1 == i2);
    BOOST_CHECK(bytes1 == bytes2);
    BOOST_CHECK(ll1 == ll2);
    BOOST_CHECK(arr1 == arr2);
    BOOST_CHECK(str_t == "Combined test \n / sdf @$()29 ");
}

BOOST_AUTO_TEST_CASE(serialization_fromBytes1)
{
    base::Bytes bytes{"fjdgl 230594 @ 235^#$ <ddfGDDFlsg SADFSD4awd4"};
    
    auto b2 = base::toBytes(bytes);
    auto s = base::fromBytes<std::string>(b2);
    BOOST_CHECK(s == bytes.toString());
}


BOOST_AUTO_TEST_CASE(serialization_fromBytes2)
{
    std::string str{"fjdgl 230594 @ 235^#$ <ddfGDDFlsg SADFSD4awd4"};
    
    auto b = base::toBytes(str);
    auto s = base::fromBytes<std::string>(b);
    BOOST_CHECK(s == str);
}


BOOST_AUTO_TEST_CASE(serialization_fromBytes3)
{
    long long ll = 9875423847583;

    auto b= base::toBytes(ll);
    auto ll2 = base::fromBytes<long long>(b);
    BOOST_CHECK(ll2 == ll);
}