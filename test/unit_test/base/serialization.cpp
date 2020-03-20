#include <boost/test/unit_test.hpp>

#include "base/serialization.hpp"

#include <limits>


namespace
{


class TestSerialization
{
  public:
    TestSerialization() = default;
    TestSerialization(int value)
      : _value(value)
    {}

    static TestSerialization deserialize(base::SerializationIArchive& ia)
    {
        int val = ia.deserialize<int>();
        return TestSerialization(val);
    }

    void serialize(base::SerializationOArchive& oa) const { oa.serialize(_value); }

    int _value;
};
} // namespace

BOOST_AUTO_TEST_CASE(serialization_sanity_check1)
{
    base::SerializationOArchive oa;
    std::uint16_t s{ 0x1234 };
    oa.serialize(base::Byte{ 0x12 });
    oa.serialize(-35);
    oa.serialize(-7ll);
    oa.serialize(s);

    base::SerializationIArchive ia(oa.getBytes());
    auto a = ia.deserialize<base::Byte>();
    auto b = ia.deserialize<int>();
    auto c = ia.deserialize<long long>();
    auto d = ia.deserialize<std::uint16_t>();
    BOOST_CHECK_EQUAL(a, 0x12);
    BOOST_CHECK_EQUAL(b, -35);
    BOOST_CHECK_EQUAL(c, -7);
    BOOST_CHECK_EQUAL(d, s);
}


BOOST_AUTO_TEST_CASE(serialization_sanity_check2)
{
    const std::uint64_t aa = 0x12345678987654;
    base::Bytes bb{ 0x1, 0x3, 0x5, 0x7, 0x15 };
    const std::vector<std::string> cc{ "a", "b", "c" };

    base::SerializationOArchive oa;
    oa.serialize(aa);
    oa.serialize(bb);
    oa.serialize(cc);

    base::SerializationIArchive ia(oa.getBytes());

    auto a = ia.deserialize<std::uint64_t>();
    BOOST_CHECK_EQUAL(a, aa);

    base::Bytes b = ia.deserialize<base::Bytes>();
    BOOST_CHECK(b == bb);

    std::vector<std::string> c = ia.deserialize<std::vector<std::string>>();
    BOOST_CHECK(c == cc);
}


BOOST_AUTO_TEST_CASE(serialization_operators_input_output)
{
    base::SerializationOArchive oa;
    std::vector<char> v1{ 'f', '!', '*', 'a' };
    std::vector<int> v2{ 2200, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 77, -99976 };
    std::vector<long long> v3{ 20000000000,
                               std::numeric_limits<long long>::min(),
                               std::numeric_limits<long long>::max() };
    std::vector<unsigned char> v4;
    oa.serialize(v1);
    oa.serialize(v2);
    oa.serialize(v3);
    oa.serialize(v4);

    base::SerializationIArchive ia(oa.getBytes());
    std::vector<char> v11 = ia.deserialize<std::vector<char>>();
    std::vector<int> v22 = ia.deserialize<std::vector<int>>();
    std::vector<long long> v33 = ia.deserialize<std::vector<long long>>();
    std::vector<unsigned char> v44 = ia.deserialize<std::vector<unsigned char>>();
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
    oa.serialize(E::A);
    oa.serialize(E::C);
    oa.serialize(E::B);
    oa.serialize(E::D);

    base::SerializationIArchive ia(oa.getBytes());
    auto a = ia.deserialize<E>();
    auto b = ia.deserialize<E>();
    auto c = ia.deserialize<E>();
    auto d = ia.deserialize<E>();

    BOOST_CHECK(a == E::A);
    BOOST_CHECK(b == E::C);
    BOOST_CHECK(c == E::B);
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
    std::vector<char> v1{ 'f', '!', '*', 'a' };
    std::vector<short> v2{ 11057, std::numeric_limits<short>::min(), std::numeric_limits<short>::max(), 767 };
    std::vector<int> v3{ 2200, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 77, -99976 };
    std::vector<long long> v4{ 20000000000,
                               std::numeric_limits<long long>::min(),
                               std::numeric_limits<long long>::max() };
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
    std::vector<std::string> v1{ "34fGEk350u8Fj", "DFN#%06784 giksdf34 \n  ", "asd35%64khrtfFsp    ad03\n\n\n\n" };
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
    std::vector<E> v1{ E::A, E::C, E::D };
    std::vector<E> v2{};

    auto b1 = base::toBytes(v1);
    auto b2 = base::toBytes(v2);

    BOOST_CHECK(v1 == base::fromBytes<std::vector<E>>(b1));
    BOOST_CHECK(v2 == base::fromBytes<std::vector<E>>(b2));
}


BOOST_AUTO_TEST_CASE(serialization_vector_with_deserialize)
{
    std::vector<TestSerialization> v1, v2, v3;
    for (std::size_t i = 0; i < 9; i++) {
        v1.emplace_back(i * 3);

        v2.emplace_back(i * 5);
        v2.emplace_back(i * 7);

        v3.emplace_back(i * 11);
        v3.emplace_back(i * 13);
        v3.emplace_back(i * 17);
    }

    base::SerializationOArchive oa;
    oa.serialize(v1);
    oa.serialize(v2);
    oa.serialize(v3);

    base::SerializationIArchive ia(oa.getBytes());
    std::vector<TestSerialization> v4 = ia.deserialize<std::vector<TestSerialization>>();
    std::vector<TestSerialization> v5 = ia.deserialize<std::vector<TestSerialization>>();
    std::vector<TestSerialization> v6 = ia.deserialize<std::vector<TestSerialization>>();

    BOOST_CHECK(v1.size() == v4.size());
    BOOST_CHECK(v2.size() == v5.size());
    BOOST_CHECK(v3.size() == v6.size());
    for (std::size_t i = 0; i < v1.size(); i++) {
        BOOST_CHECK(v1[i]._value == v4[i]._value);
    }
    for (std::size_t i = 0; i < v2.size(); i++) {
        BOOST_CHECK(v2[i]._value == v5[i]._value);
    }
    for (std::size_t i = 0; i < v3.size(); i++) {
        BOOST_CHECK(v3[i]._value == v6[i]._value);
    }
}


BOOST_AUTO_TEST_CASE(serialization_pair1)
{
    std::pair<char, TestSerialization> p1{ '6', 123 };
    std::pair<char, TestSerialization> p2{ 'a', 5 };
    std::pair<char, TestSerialization> p3{ '^', 777 };

    base::SerializationOArchive oa;
    oa.serialize(p1);
    oa.serialize(p2);
    oa.serialize(p3);

    base::SerializationIArchive ia(oa.getBytes());
    std::pair<char, TestSerialization> p4 = ia.deserialize<char, TestSerialization>();
    std::pair<char, TestSerialization> p5 = ia.deserialize<char, TestSerialization>();
    std::pair<char, TestSerialization> p6 = ia.deserialize<char, TestSerialization>();

    BOOST_CHECK((p1.first == p4.first) && (p1.second._value == p4.second._value));
    BOOST_CHECK((p2.first == p5.first) && (p2.second._value == p5.second._value));
    BOOST_CHECK((p3.first == p6.first) && (p3.second._value == p6.second._value));
}


BOOST_AUTO_TEST_CASE(serialization_pair2)
{
    TestSerialization p1{ 123 };
    TestSerialization p2{ 5 };
    TestSerialization p3{ 777 };

    base::SerializationOArchive oa;
    oa.serialize(p1);
    oa.serialize(p2);
    oa.serialize(p3);

    TestSerialization p4, p5, p6;
    base::SerializationIArchive ia(oa.getBytes());
    p4 = ia.deserialize<TestSerialization>();
    p5 = ia.deserialize<TestSerialization>();
    p6 = ia.deserialize<TestSerialization>();

    BOOST_CHECK(p1._value == p4._value);
    BOOST_CHECK(p2._value == p5._value);
    BOOST_CHECK(p3._value == p6._value);
}