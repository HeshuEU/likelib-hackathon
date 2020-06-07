#include "grpc_adapter.hpp"

#include "rpc/grpc/tools.hpp"

#include "base/config.hpp"
#include "base/error.hpp"
#include "base/log.hpp"


namespace rpc::grpc
{

void Adapter::init(std::shared_ptr<BaseRpc> service)
{
    _service = std::move(service);
}


::grpc::Status Adapter::get_account(::grpc::ServerContext* context,
                                    const ::likelib::Address* request,
                                    ::likelib::AccountInfo* response)
{
    LOG_DEBUG << "received RPC get_account method call from " << context->peer();
    try {
        auto address = deserializeAddress(request);

        auto accountInfo = _service->getAccountInfo(address);

        serializeAccountInfo(accountInfo, response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "unexpected error: " << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


::grpc::Status Adapter::get_node_info(::grpc::ServerContext* context,
                                      [[maybe_unused]] const ::likelib::None* request,
                                      ::likelib::NodeInfo* response)
{
    LOG_DEBUG << "received RPC get_node_info method call from " << context->peer();
    try {
        auto info = _service->getNodeInfo();

        serializeInfo(info, response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "unexpected error: " << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


::grpc::Status Adapter::get_block_by_hash(::grpc::ServerContext* context,
                                          const ::likelib::Hash* request,
                                          ::likelib::Block* response)
{
    LOG_DEBUG << "received RPC get_block_by_hash method call from " << context->peer();
    try {
        base::Sha256 block_hash = deserializeHash(request);

        auto block = _service->getBlock(block_hash);

        serializeBlock(block, response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "unexpected error: " << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


::grpc::Status Adapter::get_block_by_number(::grpc::ServerContext* context,
                                            const ::likelib::Number* request,
                                            ::likelib::Block* response)
{
    LOG_DEBUG << "received RPC get_block method call from " << context->peer();
    try {
        auto block = _service->getBlock(request->number());

        serializeBlock(block, response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "unexpected error: " << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


::grpc::Status Adapter::get_transaction(::grpc::ServerContext* context,
                                        const ::likelib::Hash* request,
                                        ::likelib::Transaction* response)
{
    LOG_DEBUG << "received RPC get_transaction method call from " << context->peer();
    try {
        base::Sha256 transaction_hash{ base::base64Decode(request->bytes_base_64()) };

        auto tx = _service->getTransaction(transaction_hash);

        serializeTransaction(tx, response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "unexpected error: " << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


::grpc::Status Adapter::push_transaction(::grpc::ServerContext* context,
                                         const ::likelib::Transaction* request,
                                         ::likelib::TransactionStatus* response)
{
    LOG_DEBUG << "received RPC push_transaction method call from " << context->peer();
    try {
        auto tx = deserializeTransaction(request);

        auto result = _service->pushTransaction(tx);

        serializeTransactionStatus(result, response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "unexpected error: " << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


::grpc::Status Adapter::get_transaction_result(::grpc::ServerContext* context,
                                               const ::likelib::Hash* request,
                                               ::likelib::TransactionStatus* response)
{
    LOG_DEBUG << "received RPC get_transaction_result method call from " << context->peer();
    try {
        base::Sha256 transaction_hash = deserializeHash(request);

        auto result = _service->getTransactionStatus(transaction_hash);

        serializeTransactionStatus(result, response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "unexpected error: " << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}

} // namespace rpc::grpc
