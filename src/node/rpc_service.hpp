#pragma once

#include "bc/blockchain.hpp"

#include "rpc/base_rpc.hpp"

namespace node
{

class GeneralServerService : public rpc::BaseRpc
{
  public:
    explicit GeneralServerService(bc::Blockchain* bc);

    ~GeneralServerService() override;

    bc::Balance balance(const bc::Address& address) override;

    std::string transaction(bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address,
        const base::Time& transaction_time) override;

    std::string test(const std::string& test_request) override;

  private:
    bc::Blockchain* _bc;
};
} // namespace node