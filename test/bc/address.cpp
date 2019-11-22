#include <boost/test/unit_test.hpp>

#include "bc/address.hpp"


BOOST_AUTO_TEST_CASE(address_serialization_test)
{
    bc::Address target_address(base::Sha256::calcSha256(base::Bytes("sdgfdbhd")));
    auto serialized_target = target_address.toString();
    bc::Address deserialized_address(serialized_target);
    BOOST_CHECK_EQUAL(serialized_target, deserialized_address.toString());
    BOOST_CHECK(target_address == deserialized_address);

}
