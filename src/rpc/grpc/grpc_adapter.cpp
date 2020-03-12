#include "grpc_adapter.hpp"


namespace
{

void convert(const rpc::OperationStatus& source, likelib::OperationStatus* target)
{
    target->set_message(source.getMessage());
    switch(source.getStatus()) {
        case rpc::OperationStatus::StatusCode::Success:
            target->set_status(likelib::OperationStatus_StatusCode_Success);
            break;
        case rpc::OperationStatus::StatusCode::Rejected:
            target->set_status(likelib::OperationStatus_StatusCode_Rejected);
            break;
        case rpc::OperationStatus::StatusCode::Failed:
            target->set_status(likelib::OperationStatus_StatusCode_Failed);
            break;
        default:
            RAISE_ERROR(base::LogicError, "Unexpected status code");
    }
}

} // namespace


namespace rpc
{

void GrpcAdapter::init(std::shared_ptr<BaseRpc> service)
{
    _service = service;
}


grpc::Status GrpcAdapter::test(
    grpc::ServerContext* context, const likelib::TestRequest* request, likelib::TestResponse* response)
{
    LOG_DEBUG << "received RPC call at test method from: " << context->peer();
    try {
        auto status = _service->test(request->interface_version());
        convert(status, response->mutable_status());
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}


grpc::Status GrpcAdapter::balance(
    grpc::ServerContext* context, const likelib::Address* request, likelib::CurrencyAmount* response)
{
    LOG_DEBUG << "received RPC call at balance method from: " << context->peer();
    try {
        bc::Address query_address{request->address()};
        response->set_value(_service->balance(query_address));
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


grpc::Status GrpcAdapter::info(
    grpc::ServerContext* context, const likelib::InfoRequest*, likelib::InfoResponse* response)
{
    LOG_DEBUG << "received RPC call at balance method from: " << context->peer();
    try {
        response->set_top_block_hash(base::base64Encode(_service->info().top_block_hash.getBytes()));
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


grpc::Status GrpcAdapter::get_block(
    grpc::ServerContext* context, const likelib::GetBlockRequest* request, likelib::GetBlockResponse* response)
{
    LOG_DEBUG << "received RPC get_block method call from " << context->peer();
    try {
        base::Sha256 block_hash{base::fromHex<base::Bytes>(request->block_hash())};
        auto block = _service->get_block(block_hash);
        response->set_depth(block.getDepth());
        response->set_nonce(block.getNonce());
        response->set_previous_block_hash(block.getPrevBlockHash().toHex());
        response->mutable_coinbase()->set_address(block.getCoinbase().toString());
        response->mutable_timestamp()->set_since_epoch(block.getTimestamp().getSecondsSinceEpochBeginning());

        for(const auto& tx: block.getTransactions()) {
            likelib::Transaction tv;
            tv.mutable_from()->set_address(tx.getFrom().toString());
            tv.mutable_to()->set_address(tx.getTo().toString());
            tv.mutable_value()->set_value(tx.getAmount());
            tv.mutable_gas()->set_value(tx.getFee());
            tv.mutable_creation_time()->set_since_epoch(tx.getTimestamp().getSecondsSinceEpochBeginning());
            tv.set_data(base::base64Encode(tx.getData()));
            tv.set_signature(tx.getSign().toBase64());
            response->mutable_transactions()->Add(std::move(tv));
        }
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


::grpc::Status GrpcAdapter::create_contract(::grpc::ServerContext* context,
    const ::likelib::TransactionCreateContractRequest* request, ::likelib::TransactionCreateContractResponse* response)
{
    LOG_DEBUG << "received RPC call at transaction_for_create_contract method from: " << context->peer();
    try {
        auto amount = bc::Balance{request->value().value()};
        auto from_address = bc::Address{request->from().address()};
        auto gas = bc::Balance{request->fee().value()};
        auto creation_time = base::Time::fromSecondsSinceEpochBeginning(request->creation_time().since_epoch());
        auto contract_code = request->contract_code();
        auto init = request->init();
        auto sign = bc::Sign::fromBase64(request->signature().raw());

        auto [status, contract_address, least_gas] =
            _service->transaction_create_contract(amount, from_address, creation_time, gas, contract_code, init, sign);

        convert(status, response->mutable_status());
        response->mutable_contract_address()->set_address(contract_address.toString());
        response->mutable_gas_left()->set_value(least_gas);
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}


grpc::Status GrpcAdapter::message_call(grpc::ServerContext* context,
    const likelib::TransactionMessageCallRequest* request, likelib::TransactionMessageCallResponse* response)
{
    LOG_DEBUG << "received RPC call at transaction_to_contract method from: " << context->peer();
    try {
        auto amount = bc::Balance{request->value().value()};
        auto from_address = bc::Address{request->from().address()};
        auto to_address = bc::Address{request->to().address()};
        auto gas = bc::Balance{request->fee().value()};
        auto creation_time = base::Time::fromSecondsSinceEpochBeginning(request->creation_time().since_epoch());
        auto data = request->data();
        auto sign = bc::Sign::fromBase64(request->signature());

        auto status = OperationStatus::createSuccess();
        std::string contract_response;
        bc::Balance least_gas;
        std::tie(status, contract_response, least_gas) =
            _service->transaction_message_call(amount, from_address, to_address, creation_time, gas, data, sign);

        convert(status, response->mutable_status());
        response->set_contract_response(contract_response);
        response->mutable_gas_left()->set_value(least_gas);
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}

} // namespace rpc
