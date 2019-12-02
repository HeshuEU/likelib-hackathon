#include <boost/test/unit_test.hpp>

#include "bc/address.hpp"


BOOST_AUTO_TEST_CASE(address_to_string_test)
{
    bc::Address target_address("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    auto serialized_target = target_address.toString();
    bc::Address deserialized_address(serialized_target);
    BOOST_CHECK_EQUAL(serialized_target, deserialized_address.toString());
    BOOST_CHECK(target_address == deserialized_address);
}


BOOST_AUTO_TEST_CASE(address_serialization_test)
{
    bc::Address a1("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    bc::Address a2 = base::fromBytes<bc::Address>(base::toBytes(a1));
    BOOST_CHECK(a1 == a2);
}