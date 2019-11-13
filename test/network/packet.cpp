#include <boost/test/unit_test.hpp>

#include "network/packet.hpp"

BOOST_AUTO_TEST_CASE(packet_serialization)
{
    network::Packet packet1(network::Packet::Type::DATA);
    base::Bytes serialized = packet1.serialize();
    network::Packet packet2 = network::Packet::deserialize(serialized);
    BOOST_CHECK(packet1 == packet2);
}
