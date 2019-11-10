#include <boost/test/unit_test.hpp>

#include "rpc/base_server.hpp"
#include "rpc/base_client.hpp"


class TestServer : public rpc::BaseServer
{
  public:
    bc::Balance balance(const bc::Address& address) override
    {
        return 1;
    }

    std::string transaction(bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address) override
    {
        return "test";
    }
};


class TestClient : public rpc::BaseClient
{
  public:
    explicit TestClient(TestServer* server) : _server(server)
    {}

    bc::Balance balance(const bc::Address& address) override
    {
        return _server->balance(address);
    }

    std::string transaction(bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address) override
    {
        return _server->transaction(amount, from_address, to_address);
    }

  private:
    TestServer* const _server;
};


BOOST_AUTO_TEST_CASE(rpc_loop)
{
    TestServer server;
    TestClient client(&server);
    BOOST_CHECK_EQUAL(1, client.balance(bc::Address{}));
    BOOST_CHECK_EQUAL("test", client.transaction(1, bc::Address{}, bc::Address{}));
}
