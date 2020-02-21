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
    LOG_DEBUG << "get RPC call at test method from: " << context->peer();
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
    grpc::ServerContext* context, const likelib::Address* request, likelib::Money* response)
{
    LOG_DEBUG << "get RPC call at balance method from: " << context->peer();
    try {
        bc::Address query_address{ request->address() };
        response->set_money(_service->balance(query_address));
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}


grpc::Status GrpcAdapter::transaction_to_contract(grpc::ServerContext* context,
    const likelib::TransactionToContractRequest* request, likelib::TransactionToContractResponse* response)
{
    LOG_DEBUG << "get RPC call at transaction_to_contract method from: " << context->peer();
    try {
        auto amount = bc::Balance{request->value().money()};
        auto from_address = bc::Address{request->from().address()};
        auto to_address = bc::Address{request->to().address()};
        auto gas = bc::Balance{request->gas().money()};
        auto creation_time = base::Time::fromSecondsSinceEpochBeginning(request->creation_time().since_epoch());
        auto message = base::base64Decode(request->contract_request().message());

        auto status = OperationStatus::createSuccess();
        base::Bytes contract_response;
        bc::Balance least_gas;
        std::tie(status, contract_response, least_gas) =
            _service->transaction_to_contract(amount, from_address, to_address, creation_time, gas, message, bc::Sign{});

        convert(status, response->mutable_status());
        response->mutable_contract_response()->set_message(base::base64Encode(contract_response));
        response->mutable_gas_left()->set_money(least_gas);
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}


::grpc::Status GrpcAdapter::transaction_for_create_contract(::grpc::ServerContext* context,
    const ::likelib::TransactionCreationContractRequest* request,
    ::likelib::TransactionCreationContractResponse* response)
{
    LOG_DEBUG << "get RPC call at transaction_for_create_contract method from: " << context->peer();
    try {
        auto amount = bc::Balance{request->value().money()};
        auto from_address = bc::Address{request->from().address()};
        auto gas = bc::Balance{request->gas().money()};
        auto creation_time = base::Time::fromSecondsSinceEpochBeginning(request->creation_time().since_epoch());
        auto contract_code = base::base64Decode(request->contract().bytecode());
        auto initial_message = base::base64Decode(request->initial_message().message());

        auto status = OperationStatus::createSuccess();
        bc::Address contract_address;
        bc::Balance least_gas;
        std::tie(status, contract_address, least_gas) = _service->transaction_creation_contract(
            amount, from_address, creation_time, gas, contract_code, initial_message, bc::Sign{});

        convert(status, response->mutable_status());
        response->mutable_contract_address()->set_address(contract_address.toString());
        response->mutable_gas_left()->set_money(least_gas);
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}


grpc::Status GrpcAdapter::transaction_to_wallet(grpc::ServerContext* context,
    const likelib::TransactionToAccountRequest* request, likelib::TransactionToAccountResponse* response)
{
    LOG_DEBUG << "get RPC call at transaction_to_wallet method from: " << context->peer();
    try {
        auto amount = bc::Balance{request->value().money()};
        auto from_address = bc::Address{request->from().address()};
        auto to_address = bc::Address{request->to().address()};
        auto fee = bc::Balance{request->fee().money()};
        auto creation_time = base::Time::fromSecondsSinceEpochBeginning(request->creation_time().since_epoch());
	    auto signature = base::fromBytes<bc::Sign>(base::base64Decode(request->signature().signature()));

        auto status = _service->transaction_to_wallet(amount, from_address, to_address, fee, creation_time, signature);

        convert(status, response->mutable_status());
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}

} // namespace rpc
