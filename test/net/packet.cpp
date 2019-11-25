#include <boost/test/unit_test.hpp>

#include "net/packet.hpp"

BOOST_AUTO_TEST_CASE(packet_serialization)
{
    net::Packet packet1(net::PacketType::DATA);
    base::Bytes serialized = base::toBytes(packet1);
    net::Packet packet2 = base::fromBytes<net::Packet>(serialized);
    BOOST_CHECK(packet1 == packet2);
}
