#pragma once

#include "bc/base_service.hpp"

namespace node
{

class GeneralServerService : public bc::BaseService
{
  public:
    explicit GeneralServerService();

    ~GeneralServerService() override;

    bc::Balance balance(const bc::Address& address) override;

    std::string transaction(
        bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address) override;

    std::string test(const std::string& test_request) override;
};
} // namespace node