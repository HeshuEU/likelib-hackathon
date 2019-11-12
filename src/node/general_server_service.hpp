#pragma once

#include "rpc/base_service.hpp"

namespace rpc {

    class GeneralServerService : public BaseService {
    public:
        explicit GeneralServerService();

        ~GeneralServerService() override;

        void init() override;

        bc::Balance balance(const bc::Address &address) override;

        std::string transaction(bc::Balance amount, const bc::Address &from_address,
                                const bc::Address &to_address) override;
    };
}