#include <boost/test/unit_test.hpp>

#include "core/address.hpp"

BOOST_AUTO_TEST_CASE(address_null)
{
    auto null = lk::Address::null();
    BOOST_CHECK(null.isNull());
}


BOOST_AUTO_TEST_CASE(address_constructor_fromPublicKey)
{
    lk::Address target_address(base::Secp256PrivateKey().toPublicKey());
}


BOOST_AUTO_TEST_CASE(address_constructor_from_one_publickey)
{
    auto priv_key = base::Secp256PrivateKey();
    lk::Address address1(priv_key.toPublicKey());
    lk::Address address2(priv_key.toPublicKey());
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_constructor_from_string)
{
    auto priv_key = base::Secp256PrivateKey();
    lk::Address address1(priv_key.toPublicKey());
    lk::Address address2(address1.toString());
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_constructor_from_bytes)
{
    auto priv_key = base::Secp256PrivateKey();
    lk::Address address1(priv_key.toPublicKey());
    lk::Address address2(address1.getBytes());
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_constructor_copy)
{
    auto priv_key = base::Secp256PrivateKey();
    lk::Address address1(priv_key.toPublicKey());
    lk::Address address2(address1);
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_constructor_move)
{
    auto priv_key = base::Secp256PrivateKey();
    lk::Address address(priv_key.toPublicKey());
    lk::Address address1(priv_key.toPublicKey());
    lk::Address address2(std::move(address1));
    BOOST_CHECK(address2 == address);
    BOOST_CHECK(address2.toString() == address.toString());
}


BOOST_AUTO_TEST_CASE(address_operator_equal)
{
    auto priv_key1 = base::Secp256PrivateKey();
    auto priv_key2 = base::Secp256PrivateKey();
    lk::Address address1(priv_key1.toPublicKey());
    lk::Address address2(priv_key2.toPublicKey());
    BOOST_CHECK(address1.toString() != address2.toString());
    address2 = address1;
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_operator_move)
{
    auto priv_key1 = base::Secp256PrivateKey();
    auto priv_key2 = base::Secp256PrivateKey();
    lk::Address address(priv_key1.toPublicKey());
    lk::Address address1(priv_key1.toPublicKey());
    lk::Address address2(priv_key2.toPublicKey());
    BOOST_CHECK(address1.toString() != address2.toString());
    address2 = std::move(address1);
    BOOST_CHECK(address2 == address);
    BOOST_CHECK(address2.toString() == address.toString());
}


BOOST_AUTO_TEST_CASE(address_serialization1)
{
    auto priv_key = base::Secp256PrivateKey();
    lk::Address address1(priv_key.toPublicKey());
    base::SerializationOArchive oa;
    oa.serialize(address1);
    base::SerializationIArchive ia(oa.getBytes());
    lk::Address address2 = ia.deserialize<lk::Address>();
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_serialization2)
{
    auto priv_key = base::Secp256PrivateKey();
    lk::Address a1(priv_key.toPublicKey());
    lk::Address a2 = base::fromBytes<lk::Address>(base::toBytes(a1));
    BOOST_CHECK(a1 == a2);
}
