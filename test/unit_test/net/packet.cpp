#include <boost/test/unit_test.hpp>

#include "net/packet.hpp"

BOOST_AUTO_TEST_CASE(packet_default_constructor)
{
    net::Packet packet1;
    net::Packet packet2;

    BOOST_CHECK(packet1 == packet2);
    BOOST_CHECK(packet1.getType() == packet2.getType());
}


BOOST_AUTO_TEST_CASE(packet_constructor)
{
    net::Packet packet1(net::PacketType::PING);
    net::Packet packet2(net::PacketType::DATA);
    net::Packet packet3(net::PacketType::DISCONNECT);

    BOOST_CHECK(packet1.getType() == net::PacketType::PING);
    BOOST_CHECK(packet2.getType() == net::PacketType::DATA);
    BOOST_CHECK(packet3.getType() == net::PacketType::DISCONNECT);
}


BOOST_AUTO_TEST_CASE(packet_set_and_get_data)
{
    net::Packet packet1(net::PacketType::PING);
    net::Packet packet2(net::PacketType::DISCOVERY_RES);
    net::Packet packet3(net::PacketType::DISCOVERY_REQ);
    base::Bytes bytes1("|SD,wt35/n F{eg\n dflp3DSFP#%");
    base::Bytes bytes2("(#%Fje,?249^-2=$kgm@$k6/q)");

    packet1.setData(bytes1);
    packet2.setData(bytes2);
    packet3.setData(base::Bytes(bytes1.toString() + bytes2.toString()));

    BOOST_CHECK(packet1.getData() == bytes1);
    BOOST_CHECK(packet2.getData() == bytes2);
    BOOST_CHECK(packet3.getData() == base::Bytes(bytes1.toString() + bytes2.toString()));
}


BOOST_AUTO_TEST_CASE(packet_operators_equal_and_not_equal)
{
    net::Packet packet1(net::PacketType::PONG);
    net::Packet packet2(net::PacketType::PING);
    net::Packet packet3(net::PacketType::PONG);
    base::Bytes bytes1("|SD,wt35/n F{eg\n dflp3DSFP#%");
    base::Bytes bytes2("(#%Fje,?249^-2=$kgm@$k6/q)");

    packet1.setData(bytes1);
    packet2.setData(bytes1);
    packet3.setData(bytes2);

    BOOST_CHECK(packet1 != packet2);
    BOOST_CHECK(packet1 == packet3);
    BOOST_CHECK(packet3 != packet2);
}


BOOST_AUTO_TEST_CASE(packet_serialization1)
{
    net::Packet packet1(net::PacketType::DATA);
    base::Bytes serialized = base::toBytes(packet1);
    net::Packet packet2 = base::fromBytes<net::Packet>(serialized);
    BOOST_CHECK(packet1 == packet2);
}


BOOST_AUTO_TEST_CASE(packet_serialization2)
{
    net::Packet packet1(net::PacketType::PING);
    net::Packet packet2(net::PacketType::DISCOVERY_RES);
    net::Packet packet3(net::PacketType::DISCOVERY_REQ);
    base::Bytes bytes1("|SD,wt35/n F{eg\n dflp3DSFP#%");
    base::Bytes bytes2("(#%Fje,?249^-2=$kgm@$k6/q)");

    packet1.setData(bytes1);
    packet2.setData(bytes2);
    packet3.setData(base::Bytes(bytes1.toString() + bytes2.toString()));

    base::SerializationOArchive oa;
    oa << packet1 << packet2 << packet3;
    base::SerializationIArchive ia(oa.getBytes());
    net::Packet packet11, packet22, packet33;
    ia >> packet11 >> packet22 >> packet33;

    BOOST_CHECK(packet1 == packet11);
    BOOST_CHECK(packet2 == packet22);
    BOOST_CHECK(packet3 == packet33);
    BOOST_CHECK(packet1.getType() == packet11.getType());
    BOOST_CHECK(packet2.getType() == packet22.getType());
    BOOST_CHECK(packet3.getType() == packet33.getType());
    BOOST_CHECK(packet1.getData() == packet11.getData());
    BOOST_CHECK(packet2.getData() == packet22.getData());
    BOOST_CHECK(packet3.getData() == packet33.getData());
}
