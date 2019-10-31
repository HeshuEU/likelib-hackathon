#include <boost/test/unit_test.hpp>

#include "rpc/base_rpc_server.hpp"
#include "rpc/base_rpc_client.hpp"


class TestServer : public BaseServer
{
  public:
    base::BalanceIntType balance(const base::Address& address) override
    {
        return 1;
    }

    std::string transaction(base::BalanceIntType amount, const base::Address& from_address,
                            const base::Address& to_address) override
    {
        return "test";
    }
};


class TestClient : public rpc::BaseClient
{
  public:
    explicit TestClient(TestServer* const server) : _server(server)
    {}

    base::BalanceIntType balance(const base::Address& address) override
    {
        return _server->balance(address);
    }

    std::string transaction(base::BalanceIntType amount, const base::Address& from_address,
                            const base::Address& to_address) override
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
    BOOST_CHECK_EQUAL(1, client.balance(base::Address{}));
    BOOST_CHECK_EQUAL("test", client.transaction(1, base::Address{}, base::Address{}));
}
