#include "grpc_node_client.hpp"

#include "rpc/error.hpp"

namespace rpc
{

namespace
{

    void fill_time(const base::Time& source_time, likelib::Time* target_time)
    {
        auto time_point = source_time.toTimePoint();
        time_t tt = std::chrono::system_clock::to_time_t(time_point);
        tm utc_tm = *gmtime(&tt);

        target_time->set_year_grigorian(utc_tm.tm_year + 1900);
        target_time->set_month_grigorian(utc_tm.tm_mon + 1);
        target_time->set_day_grigorian(utc_tm.tm_mday);
        target_time->set_hour_utc0(utc_tm.tm_hour);
        target_time->set_minute_utc0(utc_tm.tm_min);
        target_time->set_second_utc0(utc_tm.tm_sec);
    }

    rpc::OperationStatus convert(const ::likelib::OperationStatus& status)
    {
        switch(status.status()) {
            case ::likelib::OperationStatus_StatusCode_Success:
                return OperationStatus::createSuccess(status.message());
            case ::likelib::OperationStatus_StatusCode_Rejected:
                return OperationStatus::createRejected(status.message());
            case ::likelib::OperationStatus_StatusCode_Failed:
                return OperationStatus::createFailed(status.message());
            default:
                RAISE_ERROR(base::ParsingError, "Unexpected status code");
        }
    }


} // namespace

rpc::GrpcNodeClient::GrpcNodeClient(const std::string& connect_address)
{
    auto channel_credentials = grpc::InsecureChannelCredentials();
    _stub =
        std::make_unique<likelib::NodePublicInterface::Stub>(grpc::CreateChannel(connect_address, channel_credentials));
}

OperationStatus GrpcNodeClient::test(uint32_t api_version)
{
    // convert data for request
    likelib::TestRequest request;
    request.set_interface_version(api_version);

    // call remote host
    likelib::TestResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->test(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        return convert(reply.status());
    }
    else {
        throw RpcError(status.error_message());
    }
}

bc::Balance GrpcNodeClient::balance(const bc::Address& address)
{
    // convert data for request
    likelib::Address request;
    request.set_address_at_hex(address.toString());

    // call remote host
    likelib::Money reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->balance(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        auto result = reply.money();
        return result;
    }
    else {
        throw RpcError(status.error_message());
    }
}

std::tuple<OperationStatus, bc::Address, bc::Balance> GrpcNodeClient::transaction_creation_contract(bc::Balance amount,
    const bc::Address& from_address, const base::Time& transaction_time, bc::Balance gas, uint32_t revision,
    const base::Bytes& code, const base::Bytes& initial_message)
{
    // convert data for request
    likelib::TransactionCreationContractRequest request;

    request.mutable_value()->set_money(static_cast<google::protobuf::uint64>(amount));

    request.mutable_from()->set_address_at_hex(from_address.toString());

    request.mutable_gas()->set_money(static_cast<google::protobuf::uint64>(gas));

    request.mutable_initial_message()->set_message_at_hex(initial_message.toHex());

    request.mutable_contract()->set_revision(revision);
    request.mutable_contract()->set_bytecode_at_hex(code.toHex());

    fill_time(transaction_time, request.mutable_creation_time());

    // call remote host
    likelib::TransactionCreationContractResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->transaction_for_create_contract(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        auto converted_status = convert(reply.status());
        auto contract_address = bc::Address{reply.contract_address_at_hex().address_at_hex()};
        auto gas_left = bc::Balance{reply.gas_left().money()};
        return {converted_status, contract_address, gas_left};
    }
    else {
        throw RpcError(status.error_message());
    }
}

std::tuple<OperationStatus, base::Bytes, bc::Balance> GrpcNodeClient::transaction_to_contract(bc::Balance amount,
    const bc::Address& from_address, const bc::Address& to_address, const base::Time& transaction_time, bc::Balance gas,
    const base::Bytes& message)
{
    // convert data for request
    likelib::TransactionToContractRequest request;

    request.mutable_value()->set_money(static_cast<google::protobuf::uint64>(amount));

    request.mutable_from()->set_address_at_hex(from_address.toString());

    request.mutable_to()->set_address_at_hex(to_address.toString());

    request.mutable_gas()->set_money(static_cast<google::protobuf::uint64>(gas));

    request.mutable_contract_request()->set_message_at_hex(message.toHex());

    fill_time(transaction_time, request.mutable_creation_time());

    // call remote host
    likelib::TransactionToContractResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->transaction_to_contract(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        auto converted_status = convert(reply.status());
        auto gas_left = bc::Balance{reply.gas_left().money()};
        auto message_from_contract = base::Bytes::fromHex(reply.contract_response().message_at_hex());
        return {converted_status, message_from_contract, gas_left};
    }
    else {
        throw RpcError(status.error_message());
    }
}

OperationStatus GrpcNodeClient::transaction_to_wallet(bc::Balance amount, const bc::Address& from_address,
    const bc::Address& to_address, const base::Time& transaction_time)
{
    // convert data for request
    likelib::TransactionToAccountRequest request;

    request.mutable_value()->set_money(static_cast<google::protobuf::uint64>(amount));

    request.mutable_from()->set_address_at_hex(from_address.toString());

    request.mutable_to()->set_address_at_hex(to_address.toString());

    fill_time(transaction_time, request.mutable_creation_time());

    // call remote host
    likelib::TransactionToAccountResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->transaction_to_wallet(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        return convert(reply.status());
    }
    else {
        throw RpcError(status.error_message());
    }
}

} // namespace rpc
