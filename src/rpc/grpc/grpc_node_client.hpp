#pragma once

#include "gen/public_rpc.grpc.pb.h"

#include "bc/base_client.hpp"

#include <grpcpp/grpcpp.h>

#include <string>

namespace rpc {

    class GrpcNodeClient final : bc::BaseClient {
    public:
        explicit GrpcNodeClient(const std::string &connect_address);

        ~GrpcNodeClient() override = default;

        bc::Balance balance(const bc::Address &address) override;

        std::string transaction(bc::Balance amount, const bc::Address &from_address,
                                const bc::Address &to_address) override;

    private:
        std::unique_ptr<Likelib::Node::Stub> _stub;
    };

}