#include "grpc_adapter.hpp"

namespace rpc
{

namespace
{

    base::Time convert(const likelib::Time& source)
    {
        tm utc_tm;
        utc_tm.tm_year = source.year_grigorian();
        utc_tm.tm_mon = source.month_grigorian();
        utc_tm.tm_mday = source.day_grigorian();
        utc_tm.tm_hour = source.hour_utc0();
        utc_tm.tm_min = source.minute_utc0();
        utc_tm.tm_sec = source.second_utc0();

        auto tt = mktime(&utc_tm);
        return base::Time::fromTimePoint(std::chrono::system_clock::from_time_t(tt));
    }

    void convert(const OperationStatus& source, likelib::OperationStatus* target)
    {
        target->set_message(source.getMessage());
        switch(source.getStatus()) {
            case OperationStatus::StatusCode::Success:
                target->set_status(likelib::OperationStatus_StatusCode_Success);
                break;
            case OperationStatus::StatusCode::Rejected:
                target->set_status(likelib::OperationStatus_StatusCode_Rejected);
                break;
            case OperationStatus::StatusCode::Failed:
                target->set_status(likelib::OperationStatus_StatusCode_Failed);
                break;
            default:
                RAISE_ERROR(base::LogicError, "Unexpected status code");
        }
    }

} // namespace

void GrpcAdapter::init(std::shared_ptr<BaseRpc> service)
{
    _service = service;
}

grpc::Status GrpcAdapter::test(
    grpc::ServerContext* context, const likelib::TestRequest* request, likelib::TestResponse* response)
{
    LOG_DEBUG << "get RPC call at test method from:" << context->peer();
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
    LOG_DEBUG << "get RPC call at balance method from:" << context->peer();
    try {
        response->set_money(_service->balance(request->address_at_hex()));
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
    LOG_DEBUG << "get RPC call at transaction_to_contract method from:" << context->peer();
    try {
        auto amount = bc::Balance{request->value().money()};
        auto from_address = bc::Address{request->from().address_at_hex()};
        auto to_address = bc::Address{request->to().address_at_hex()};
        auto gas = bc::Balance{request->gas().money()};
        auto creation_time = convert(request->creation_time());
        auto message = base::Bytes::fromHex(request->contract_request().message_at_hex());

        auto status = OperationStatus::createSuccess();
        base::Bytes contract_response;
        bc::Balance least_gas;
        std::tie(status, contract_response, least_gas) =
            _service->transaction_to_contract(amount, from_address, to_address, creation_time, gas, message);

        convert(status, response->mutable_status());
        response->mutable_contract_response()->set_message_at_hex(contract_response.toString());
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
    LOG_DEBUG << "get RPC call at transaction_for_create_contract method from:" << context->peer();
    try {
        auto amount = bc::Balance{request->value().money()};
        auto from_address = bc::Address{request->from().address_at_hex()};
        auto gas = bc::Balance{request->gas().money()};
        auto creation_time = convert(request->creation_time());
        auto contract_code = base::Bytes::fromHex(request->contract().bytecode_at_hex());
        auto code_revision = request->contract().revision();
        auto initial_message = base::Bytes::fromHex(request->initial_message().message_at_hex());

        auto status = OperationStatus::createSuccess();
        bc::Address contract_address;
        bc::Balance least_gas;
        std::tie(status, contract_address, least_gas) = _service->transaction_creation_contract(
            amount, from_address, creation_time, gas, code_revision, contract_code, initial_message);

        convert(status, response->mutable_status());
        response->mutable_contract_address_at_hex()->set_address_at_hex(contract_address.toString());
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
    LOG_DEBUG << "get RPC call at transaction_to_wallet method from:" << context->peer();
    try {
        auto amount = bc::Balance{request->value().money()};
        auto from_address = bc::Address{request->from().address_at_hex()};
        auto to_address = bc::Address{request->to().address_at_hex()};
        auto fee = bc::Balance{request->fee().money()};
        auto creation_time = convert(request->creation_time());

        auto status = _service->transaction_to_wallet(amount, from_address, to_address, fee, creation_time);

        convert(status, response->mutable_status());
    }
    catch(const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }

    return ::grpc::Status::OK;
}



} // namespace rpc