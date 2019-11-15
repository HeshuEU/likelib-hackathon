#include <boost/test/unit_test.hpp>

#include "net/packet.hpp"

BOOST_AUTO_TEST_CASE(packet_serialization)
{
    net::Packet packet1(net::Packet::Type::DATA);
    base::Bytes serialized = packet1.serialize();
    net::Packet packet2 = net::Packet::deserialize(serialized);
    BOOST_CHECK(packet1 == packet2);
}
