#include <boost/test/unit_test.hpp>

#include "rpc/base_rpc_server.hpp"
#include "rpc/base_rpc_client.hpp"

class TestServer : public BaseRpcServer
{
  public:
    base::BalanceIntType balance(const std::string& address) override
    {
        return 1;
    }

    std::string transaction(int amount, const std::string& from_address, const std::string& to_address) override
    {
        return "test";
    }
};


class TestClient : public BaseRpcClient
{
  public:
    explicit TestClient(TestServer* const server) : _server(server)
    {}

    base::BalanceIntType balance(const std::string& address) override
    {
        return _server->balance(address);
    }

    std::string transaction(int amount, const std::string& from_address, const std::string& to_address) override
    {
        return _server->transaction(amount, from_address, to_address);
    }

  private:
    TestServer* _server;
};


BOOST_AUTO_TEST_CASE(rpc_loop)
{
    TestServer server;
    TestClient client(&server);
    BOOST_CHECK_EQUAL(1, client.balance(""));
    BOOST_CHECK_EQUAL("test", client.transaction(1, "", ""));
}