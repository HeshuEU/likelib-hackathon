#include <boost/test/unit_test.hpp>

#include "bc/address.hpp"


BOOST_AUTO_TEST_CASE(address_constructor_and_toString1)
{
    bc::Address target_address("Address test 5%$&3495 // \nfg23j");
    BOOST_CHECK(target_address.toString() == "Address test 5%$&3495 // \nfg23j");
}


BOOST_AUTO_TEST_CASE(address_constructor_and_toString2)
{
    bc::Address target_address(std::string("Address test 5%$&3495 // \nfg23j"));
    BOOST_CHECK(target_address.toString() == "Address test 5%$&3495 // \nfg23j");
}


BOOST_AUTO_TEST_CASE(address_constructor_and_toString3)
{
    bc::Address address1("Address test 5%$&3495 // \nfg23j");
    auto str = address1.toString();
    bc::Address address2(str);
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == "Address test 5%$&3495 // \nfg23j");
    BOOST_CHECK(address2.toString() == "Address test 5%$&3495 // \nfg23j");
}


BOOST_AUTO_TEST_CASE(address_constructor_copy)
{
    bc::Address address1("Address test 5%$&3495 // \nfg23j");
    bc::Address address2(address1);
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == "Address test 5%$&3495 // \nfg23j");
    BOOST_CHECK(address2.toString() == "Address test 5%$&3495 // \nfg23j");
}


BOOST_AUTO_TEST_CASE(address_constructor_move)
{
    bc::Address address1("Address test 5%$&3495 // \nfg23j");
    bc::Address address2(std::move(address1));
    BOOST_CHECK(address2.toString() == "Address test 5%$&3495 // \nfg23j");
}


BOOST_AUTO_TEST_CASE(address_operator_equal)
{
    bc::Address address1("Address test 5%$&3495 // \nfg23j");
    bc::Address address2("per^&34GFKS04\n\\dsfjs/");
    BOOST_CHECK(address1.toString() != address2.toString());
    address2 = address1;
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address2.toString() == "Address test 5%$&3495 // \nfg23j");
}


BOOST_AUTO_TEST_CASE(address_operator_move)
{
    bc::Address address1("Address test 5%$&3495 // \nfg23j");
    bc::Address address2("per^&34GFKS04\n\\dsfjs/");
    BOOST_CHECK(address1.toString() != address2.toString());
    address2 = std::move(address1);
    BOOST_CHECK(address2.toString() == "Address test 5%$&3495 // \nfg23j");
}


BOOST_AUTO_TEST_CASE(address_serialization1)
{
    bc::Address address1("Address test 5%$&3495 // \nfg23j");
    base::SerializationOArchive oa;
    oa << address1;
    base::SerializationIArchive ia(oa.getBytes());
    bc::Address address2;
    ia >> address2;
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == "Address test 5%$&3495 // \nfg23j");
    BOOST_CHECK(address2.toString() == "Address test 5%$&3495 // \nfg23j");
}


BOOST_AUTO_TEST_CASE(address_serialization2)
{
    bc::Address a1("Address test 5%$&3495 // \nfg23j");
    bc::Address a2 = base::fromBytes<bc::Address>(base::toBytes(a1));
    BOOST_CHECK(a1 == a2);
}