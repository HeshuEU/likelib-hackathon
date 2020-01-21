#include <boost/test/unit_test.hpp>

#include "net/endpoint.hpp"

#include <sstream>

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


BOOST_AUTO_TEST_CASE(endpoint_operators_equal_and_not_equal)
{
    std::string address_with_port1 = "123.222.112.211:10000";
    std::string address_with_port2 = "123.222.112.211:10001";
    std::string address_with_port3 = "223.222.112.211:10000";
    net::Endpoint endpoint1(address_with_port1);
    net::Endpoint endpoint11(address_with_port1);
    net::Endpoint endpoint2(address_with_port2);
    net::Endpoint endpoint3(address_with_port3);

    BOOST_CHECK(endpoint1 == endpoint11);
    BOOST_CHECK(endpoint1 != endpoint2);
    BOOST_CHECK(endpoint1 != endpoint3);
    BOOST_CHECK(endpoint2 != endpoint3);
}


BOOST_AUTO_TEST_CASE(endpoint_operators_more_and_less)
{
    std::string address_with_port1 = "123.222.112.211:10000";
    std::string address_with_port2 = "123.222.112.211:10001";
    std::string address_with_port3 = "223.222.112.211:10000";
    net::Endpoint endpoint1(address_with_port1);
    net::Endpoint endpoint2(address_with_port2);
    net::Endpoint endpoint3(address_with_port3);

    BOOST_CHECK(endpoint1 < endpoint2);
    BOOST_CHECK(endpoint1 < endpoint3);
    BOOST_CHECK(endpoint2 < endpoint3);

    BOOST_CHECK(endpoint2 > endpoint1);
    BOOST_CHECK(endpoint3 > endpoint1);
    BOOST_CHECK(endpoint3 > endpoint2);
}


BOOST_AUTO_TEST_CASE(endpoint_operators_more_or_equal_and_less_or_equal)
{
    std::string address_with_port1 = "123.222.112.211:10000";
    std::string address_with_port2 = "123.222.112.211:10001";
    std::string address_with_port3 = "223.222.112.211:10000";
    net::Endpoint endpoint1(address_with_port1);
    net::Endpoint endpoint11(address_with_port1);
    net::Endpoint endpoint2(address_with_port2);
    net::Endpoint endpoint3(address_with_port3);

    BOOST_CHECK(endpoint1 <= endpoint11);
    BOOST_CHECK(endpoint1 >= endpoint11);

    BOOST_CHECK(endpoint1 <= endpoint2);
    BOOST_CHECK(endpoint1 <= endpoint3);
    BOOST_CHECK(endpoint2 <= endpoint3);

    BOOST_CHECK(endpoint2 >= endpoint1);
    BOOST_CHECK(endpoint3 >= endpoint1);
    BOOST_CHECK(endpoint3 >= endpoint2);
}


BOOST_AUTO_TEST_CASE(endpoint_output_operator)
{
    std::string address_with_port1 = "123.222.112.211:10000";
    std::string address_with_port2 = "0.0.0.0:0";
    std::string address_with_port3 = "255.255.255.255:65535";
    net::Endpoint endpoint1(address_with_port1);
    net::Endpoint endpoint2(address_with_port2);
    net::Endpoint endpoint3(address_with_port3);

    std::stringstream stream;
    stream << endpoint1 << endpoint2 << endpoint3;
    std::string str;
    stream >> str;

    BOOST_CHECK_EQUAL(str, address_with_port1 + address_with_port2 + address_with_port3);
}