#include <boost/test/unit_test.hpp>

#include "bc/address.hpp"

BOOST_AUTO_TEST_CASE(address_default_constructor)
{
    bc::Address address1;
    bc::Address address2;
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == "");
}

BOOST_AUTO_TEST_CASE(address_constructor_and_toString1)
{
    std::string address_str = "Address test 5%$&3495 // \nfg23j";
    bc::Address target_address(address_str);
    BOOST_CHECK(target_address.toString() == address_str);
}


BOOST_AUTO_TEST_CASE(address_constructor_and_toString2)
{
    char address_str[] = "Address test 5%$&3495 // \nfg23j";
    bc::Address target_address(address_str);
    BOOST_CHECK(target_address.toString() == std::string(address_str));
}


BOOST_AUTO_TEST_CASE(address_constructor_and_toString3)
{
    std::string address_str = "Address test 5%$&3495 // \nfg23j";
    bc::Address address1(address_str);
    auto str = address1.toString();
    bc::Address address2(str);
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address_str);
    BOOST_CHECK(address2.toString() == address_str);
}


BOOST_AUTO_TEST_CASE(address_constructor_copy)
{
    std::string address_str = "Address test 5%$&3495 // \nfg23j";
    bc::Address address1(address_str);
    bc::Address address2(address1);
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address_str);
    BOOST_CHECK(address2.toString() == address_str);
}


BOOST_AUTO_TEST_CASE(address_constructor_move)
{
    std::string address_str = "Address test 5%$&3495 // \nfg23j";
    bc::Address address1(address_str);
    bc::Address address2(std::move(address1));
    BOOST_CHECK(address2.toString() == address_str);
}


BOOST_AUTO_TEST_CASE(address_operator_equal)
{
    std::string address_str = "Address test 5%$&3495 // \nfg23j";
    bc::Address address1(address_str);
    bc::Address address2("per^&34GFKS04\n\\dsfjs/");
    BOOST_CHECK(address1.toString() != address2.toString());
    address2 = address1;
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address2.toString() == address_str);
}


BOOST_AUTO_TEST_CASE(address_operator_move)
{
    std::string address_str = "Address test 5%$&3495 // \nfg23j";
    bc::Address address1(address_str);
    bc::Address address2("per^&34GFKS04\n\\dsfjs/");
    BOOST_CHECK(address1.toString() != address2.toString());
    address2 = std::move(address1);
    BOOST_CHECK(address2.toString() == address_str);
}


BOOST_AUTO_TEST_CASE(address_serialization1)
{
    std::string address_str = "Address test 5%$&3495 // \nfg23j";
    bc::Address address1(address_str);
    base::SerializationOArchive oa;
    oa << address1;
    base::SerializationIArchive ia(oa.getBytes());
    bc::Address address2;
    ia >> address2;
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address_str);
    BOOST_CHECK(address2.toString() == address_str);
}


BOOST_AUTO_TEST_CASE(address_serialization2)
{
    bc::Address a1("Address test 5%$&3495 // \nfg23j");
    bc::Address a2 = base::fromBytes<bc::Address>(base::toBytes(a1));
    BOOST_CHECK(a1 == a2);
}