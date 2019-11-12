#pragma once

#include "gen/public_rpc.grpc.pb.h"

#include "rpc/base_service.hpp"

#include "base/log.hpp"

#include <grpcpp/grpcpp.h>

namespace rpc {

    template<typename LogicService>
    class GrpcNodeServiceImpl final : public Likelib::Node::Service {
    public:
        explicit GrpcNodeServiceImpl() : _service{} {}

        ~GrpcNodeServiceImpl() override = default;

        void init() {
            _service.init();
        }

    private:
        LogicService _service;

        grpc::Status
        balance(grpc::ServerContext *context, const Likelib::Address *request, Likelib::Money *response) override {
            logContextData(context);
            response->set_money(_service.balance(request->address().c_str()));
            return grpc::Status::OK;
        }

        grpc::Status
        transaction(grpc::ServerContext *context, const Likelib::Transaction *request,
                    Likelib::Hash *response) override {
            logContextData(context);
            auto from_address = request->from_address().address().c_str();
            auto to_address = request->to_address().address().c_str();
            auto amount = request->amount().money();
            response->set_hash_string(_service.transaction(amount, from_address, to_address));
            return grpc::Status::OK;
        }

        static void logContextData(grpc::ServerContext *context) {
            LOG_TRACE << "Peer:" << context->peer();
            LOG_TRACE << "----------- Client metadata -----------";
            for (auto &pair : context->client_metadata()) {
                LOG_TRACE << "Key[" << pair.first.data() << "], value [" << pair.second.data() << "]";
            }
        }
    };

}
