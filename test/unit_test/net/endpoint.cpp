#include <boost/test/unit_test.hpp>

#include "net/endpoint.hpp"

BOOST_AUTO_TEST_CASE(endpoint_constructor_with_one_paramater)
{
    unsigned short port = 12345;
    std::string address_with_port = "127.0.0.1:" + std::to_string(port);
    net::Endpoint endpoint(address_with_port);

    BOOST_CHECK(endpoint.toString() == address_with_port);
    BOOST_CHECK(endpoint.getPort() == port);
}


BOOST_AUTO_TEST_CASE(endpoint_constructor_with_one_paramater_min_values)
{
    unsigned short port = 0;
    std::string address_with_port = "0.0.0.0:" + std::to_string(port);
    net::Endpoint endpoint(address_with_port);

    BOOST_CHECK(endpoint.toString() == address_with_port);
    BOOST_CHECK(endpoint.getPort() == port);
}


BOOST_AUTO_TEST_CASE(endpoint_constructor_with_one_paramater_max_values)
{
    unsigned short port = 65535;
    std::string address_with_port = "255.255.255.255:" + std::to_string(port);
    net::Endpoint endpoint(address_with_port);

    BOOST_CHECK(endpoint.toString() == address_with_port);
    BOOST_CHECK(endpoint.getPort() == port);
}


BOOST_AUTO_TEST_CASE(endpoint_constructor_with_two_paramaters)
{
    unsigned short port = 12345;
    std::string address = "127.0.0.1";
    net::Endpoint endpoint(address, port);

    BOOST_CHECK_EQUAL(endpoint.toString(), address + ':' + std::to_string(port));
    BOOST_CHECK_EQUAL(endpoint.getPort(), port);
}


BOOST_AUTO_TEST_CASE(endpoint_constructor_with_two_paramaters_min_values)
{
    unsigned short port = 0;
    std::string address = "0.0.0.0";
    net::Endpoint endpoint(address, port);

    BOOST_CHECK_EQUAL(endpoint.toString(), address + ':' + std::to_string(port));
    BOOST_CHECK_EQUAL(endpoint.getPort(), port);
}


BOOST_AUTO_TEST_CASE(endpoint_constructor_with_two_paramaters_max_values)
{
    unsigned short port = 65535;
    std::string address = "255.255.255.255";
    net::Endpoint endpoint(address, port);

    BOOST_CHECK_EQUAL(endpoint.toString(), address + ':' + std::to_string(port));
    BOOST_CHECK_EQUAL(endpoint.getPort(), port);
}