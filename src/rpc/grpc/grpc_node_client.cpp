#include "grpc_node_client.hpp"

#include "rpc/error.hpp"


namespace
{

rpc::OperationStatus convert(const ::likelib::OperationStatus& status)
{
    switch(status.status()) {
        case ::likelib::OperationStatus_StatusCode_Success:
            return rpc::OperationStatus::createSuccess(status.message());
        case ::likelib::OperationStatus_StatusCode_Rejected:
            return rpc::OperationStatus::createRejected(status.message());
        case ::likelib::OperationStatus_StatusCode_Failed:
            return rpc::OperationStatus::createFailed(status.message());
        default:
            RAISE_ERROR(base::ParsingError, "Unexpected status code");
    }
}


} // namespace


namespace rpc
{

GrpcNodeClient::GrpcNodeClient(const std::string& connect_address)
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
    request.set_address(address.toString());

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


Info GrpcNodeClient::info()
{
    likelib::InfoRequest request;

    // call remote host
    likelib::InfoResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->info(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        Info ret{base::Sha256(base::base64Decode(reply.top_block_hash())), 0};
        return ret;
    }
    else {
        throw RpcError(status.error_message());
    }
}


std::tuple<OperationStatus, bc::Address, bc::Balance> GrpcNodeClient::transaction_create_contract(bc::Balance amount,
    const bc::Address& from_address, const base::Time& transaction_time, bc::Balance gas,
    const std::string& contract_code, const std::string& init, const bc::Sign& signature)
{
    // convert data for request
    likelib::TransactionCreateContractRequest request;
    request.mutable_value()->set_money(static_cast<google::protobuf::uint64>(amount));
    request.mutable_from()->set_address(from_address.toString());
    request.mutable_gas()->set_money(static_cast<google::protobuf::uint64>(gas));
    request.set_init(init);
    request.set_contract_code(contract_code);
    request.mutable_creation_time()->set_since_epoch(transaction_time.getSecondsSinceEpochBeginning());
    request.set_signature(signature.toBase64());

    // call remote host
    likelib::TransactionCreateContractResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->create_contract(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        auto converted_status = convert(reply.status());
        auto contract_address = bc::Address{reply.contract_address().address()};
        auto gas_left = bc::Balance{reply.gas_left().money()};
        return {converted_status, contract_address, gas_left};
    }
    else {
        throw RpcError(status.error_message());
    }
}

std::tuple<OperationStatus, std::string, bc::Balance> GrpcNodeClient::transaction_message_call(bc::Balance amount,
    const bc::Address& from_address, const bc::Address& to_address, const base::Time& transaction_time, bc::Balance gas,
    const std::string& data, const bc::Sign& signature)
{
    // convert data for request
    likelib::TransactionMessageCallRequest request;
    request.mutable_value()->set_money(static_cast<google::protobuf::uint64>(amount));
    request.mutable_from()->set_address(from_address.toString());
    request.mutable_to()->set_address(to_address.toString());
    request.mutable_gas()->set_money(static_cast<google::protobuf::uint64>(gas));
    request.set_data(data);
    request.mutable_creation_time()->set_since_epoch(transaction_time.getSecondsSinceEpochBeginning());
    request.set_signature(signature.toBase64());

    // call remote host
    likelib::TransactionMessageCallResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->message_call(&context, request, &reply);

    // return value if ok
    if(status.ok()) {
        auto converted_status = convert(reply.status());
        auto gas_left = bc::Balance{reply.gas_left().money()};
        auto message_from_contract = reply.contract_response();
        return {converted_status, message_from_contract, gas_left};
    }
    else {
        throw RpcError(status.error_message());
    }
}

} // namespace rpc
