#include <boost/test/unit_test.hpp>

#include "base/serialization.hpp"

BOOST_AUTO_TEST_CASE(serialization_sanity_check1)
{
    base::SerializationOArchive oa;
    std::uint16_t s{0x1234};
    oa << base::Byte{0x12} << -35 << -7ll << s;

    base::SerializationIArchive ia(oa.getBytes());
    base::Byte a;
    int b;
    long long c;
    std::uint16_t d;
    ia >> a >> b >> c >> d;
    BOOST_CHECK_EQUAL(a, 0x12);
    BOOST_CHECK_EQUAL(b, -35);
    BOOST_CHECK_EQUAL(c, -7);
    BOOST_CHECK_EQUAL(d, s);
}


BOOST_AUTO_TEST_CASE(serialization_sanity_check2)
{
    const std::uint64_t aa = 0x12345678987654;
    base::Bytes bb{0x1, 0x3, 0x5, 0x7, 0x15};
    const std::vector<std::string> cc{"a", "b", "c"};

    base::SerializationOArchive oa;
    oa << aa << bb << cc;

    base::SerializationIArchive ia(oa.getBytes());

    std::uint64_t a;
    ia >> a;
    BOOST_CHECK_EQUAL(a, aa);

    base::Bytes b;
    ia >> b;
    BOOST_CHECK(b == bb);

    std::vector<std::string> c;
    ia >> c;
    BOOST_CHECK(c == cc);
}


BOOST_AUTO_TEST_CASE(serialization_operators_input_output)
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


BOOST_AUTO_TEST_CASE(serialization_enum)
{
    enum class E {
        A, B, C, D
    };
    base::SerializationOArchive oa;
    oa << E::A << E::C << E::B << E::D;

    E a, b, c, d;
    base::SerializationIArchive ia(oa.getBytes());
    ia >> a >> c >> b >> d;

    BOOST_CHECK(a == E::A);
    BOOST_CHECK(b == E::B);
    BOOST_CHECK(c == E::C);
    BOOST_CHECK(d == E::D);
}