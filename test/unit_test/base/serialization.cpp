#include <boost/test/unit_test.hpp>

#include "base/serialization.hpp"

#include <limits>


namespace
{


class TestDeserializeClass
{
  public:
    TestDeserializeClass() = default;
    TestDeserializeClass(int value) : _value(value)
    {}

    static TestDeserializeClass deserialize(base::SerializationIArchive& ia)
    {
        int val;
        ia >> val;
        return TestDeserializeClass(val);
    }

    int _value;
};

base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const TestDeserializeClass& obj)
{
    return oa << obj._value;
}
} // namespace

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
    std::vector<int> v2{2200, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 77, -99976};
    std::vector<long long> v3{
        20000000000, std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max()};
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
    enum class E
    {
        A,
        B,
        C,
        D
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


BOOST_AUTO_TEST_CASE(serialization_toBytes_fromBytes_integers)
{
    char ch = 98;
    int short sh = 30111;
    int i = 57678;
    int long long ll = 5471756785678;

    auto bch = base::toBytes(ch);
    auto bsh = base::toBytes(sh);
    auto bi = base::toBytes(i);
    auto bll = base::toBytes(ll);

    BOOST_CHECK_EQUAL(ch, base::fromBytes<char>(bch));
    BOOST_CHECK_EQUAL(sh, base::fromBytes<short>(bsh));
    BOOST_CHECK_EQUAL(i, base::fromBytes<int>(bi));
    BOOST_CHECK_EQUAL(ll, base::fromBytes<long long>(bll));
}


BOOST_AUTO_TEST_CASE(serialization_toBytes_fromBytes_strings)
{
    std::string str1 = "fgkj3%835/ n af[dsgp/n   ";
    std::string str2 = "*(#%JGold^LF\n sdei6^94";

    auto b1 = base::toBytes(str1);
    auto b2 = base::toBytes(str2);

    BOOST_CHECK_EQUAL(str1, base::fromBytes<std::string>(b1));
    BOOST_CHECK_EQUAL(str2, base::fromBytes<std::string>(b2));
}


BOOST_AUTO_TEST_CASE(serialization_toBytes_fromBytes_vectors_integers)
{
    std::vector<char> v1{'f', '!', '*', 'a'};
    std::vector<short> v2{11057, std::numeric_limits<short>::min(), std::numeric_limits<short>::max(), 767};
    std::vector<int> v3{2200, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 77, -99976};
    std::vector<long long> v4{
        20000000000, std::numeric_limits<long long>::min(), std::numeric_limits<long long>::max()};
    std::vector<unsigned char> v5;

    auto b1 = base::toBytes(v1);
    auto b2 = base::toBytes(v2);
    auto b3 = base::toBytes(v3);
    auto b4 = base::toBytes(v4);
    auto b5 = base::toBytes(v5);

    BOOST_CHECK(v1 == base::fromBytes<std::vector<char>>(b1));
    BOOST_CHECK(v2 == base::fromBytes<std::vector<short>>(b2));
    BOOST_CHECK(v3 == base::fromBytes<std::vector<int>>(b3));
    BOOST_CHECK(v4 == base::fromBytes<std::vector<long long>>(b4));
    BOOST_CHECK(v5 == base::fromBytes<std::vector<unsigned char>>(b5));
}


BOOST_AUTO_TEST_CASE(serialization_toBytes_fromBytes_vectors_strings)
{
    std::vector<std::string> v1{"34fGEk350u8Fj", "DFN#%06784 giksdf34 \n  ", "asd35%64khrtfFsp    ad03\n\n\n\n"};
    std::vector<std::string> v2{};

    auto b1 = base::toBytes(v1);
    auto b2 = base::toBytes(v2);

    BOOST_CHECK(v1 == base::fromBytes<std::vector<std::string>>(b1));
    BOOST_CHECK(v2 == base::fromBytes<std::vector<std::string>>(b2));
}


BOOST_AUTO_TEST_CASE(serialization_toBytes_fromBytes_vectors_enum)
{
    enum class E
    {
        A,
        B,
        C,
        D
    };
    std::vector<E> v1{E::A, E::C, E::D};
    std::vector<E> v2{};

    auto b1 = base::toBytes(v1);
    auto b2 = base::toBytes(v2);

    BOOST_CHECK(v1 == base::fromBytes<std::vector<E>>(b1));
    BOOST_CHECK(v2 == base::fromBytes<std::vector<E>>(b2));
}


BOOST_AUTO_TEST_CASE(serialization_vector_with_deserialize)
{
    std::vector<TestDeserializeClass> v1, v2, v3;
    for(std::size_t i = 0; i < 9; i++) {
        v1.emplace_back(i * 3);

        v2.emplace_back(i * 5);
        v2.emplace_back(i * 7);

        v3.emplace_back(i * 11);
        v3.emplace_back(i * 13);
        v3.emplace_back(i * 17);
    }

    base::SerializationOArchive oa;
    oa << v1 << v2 << v3;

    std::vector<TestDeserializeClass> v4, v5, v6;
    base::SerializationIArchive ia(oa.getBytes());
    ia >> v4 >> v5 >> v6;

    BOOST_CHECK(v1.size() == v4.size());
    BOOST_CHECK(v2.size() == v5.size());
    BOOST_CHECK(v3.size() == v6.size());
    for(std::size_t i = 0; i < v1.size(); i++) {
        BOOST_CHECK(v1[i]._value == v4[i]._value);
    }
    for(std::size_t i = 0; i < v2.size(); i++) {
        BOOST_CHECK(v2[i]._value == v5[i]._value);
    }
    for(std::size_t i = 0; i < v3.size(); i++) {
        BOOST_CHECK(v3[i]._value == v6[i]._value);
    }
}