#include "grpc_node_client.hpp"

#include "rpc/error.hpp"


namespace
{

rpc::OperationStatus convert(const ::likelib::OperationStatus& status)
{
    switch (status.status()) {
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
    if (status.ok()) {
        return convert(reply.status());
    }
    else {
        throw RpcError(status.error_message());
    }
}

std::string GrpcNodeClient::balance(const lk::Address& address)
{
    // convert data for request
    likelib::Address request;
    request.set_address(address.toString());

    // call remote host
    likelib::CurrencyAmount reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->balance(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        auto result = reply.value();
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
    if (status.ok()) {
        Info ret{ base::Sha256(base::base64Decode(reply.top_block_hash())), 0 };
        return ret;
    }
    else {
        throw RpcError(status.error_message());
    }
}


lk::Block GrpcNodeClient::get_block(const base::Sha256& block_hash)
{
    likelib::GetBlockRequest request;
    request.set_block_hash(block_hash.toHex());

    // call remote host
    likelib::GetBlockResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->get_block(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        lk::BlockDepth depth{ reply.depth() };
        base::Sha256 prev_block_hash{ base::fromHex<base::Bytes>(reply.previous_block_hash()) };
        auto timestamp = base::Time(reply.timestamp().since_epoch());
        lk::NonceInt nonce{ reply.nonce() };
        lk::Address coinbase{ reply.coinbase().address() };

        lk::TransactionsSet txset;
        for (const auto& txv : reply.transactions()) {
            lk::TransactionBuilder txb;
            txb.setFrom(lk::Address(txv.from().address()));
            txb.setTo(lk::Address(txv.to().address()));
            txb.setAmount(txv.value().value());
            txb.setFee(txv.fee().value());
            txb.setTimestamp(base::Time(txv.creation_time().since_epoch()));
            txb.setData(base::base64Decode(txv.data()));
            txb.setSign(lk::Sign::fromBase64(txv.signature()));
            txb.setType(lk::Address(txv.to().address()) == lk::Address::null() ?
                          lk::Transaction::Type::CONTRACT_CREATION :
                          lk::Transaction::Type::MESSAGE_CALL);
            txset.add(std::move(txb).build());
        }

        lk::Block blk{ depth, std::move(prev_block_hash), timestamp, std::move(coinbase), std::move(txset) };
        blk.setNonce(nonce);
        return blk;
    }
    else {
        throw RpcError(status.error_message());
    }
}


std::tuple<OperationStatus, lk::Address, std::uint64_t> GrpcNodeClient::transaction_create_contract(
  lk::Balance amount,
  const lk::Address& from_address,
  const base::Time& transaction_time,
  std::uint64_t gas,
  const std::string& contract_code,
  const std::string& init,
  const lk::Sign& signature)
{
    // convert data for request
    likelib::TransactionCreateContractRequest request;
    request.mutable_value()->set_value(static_cast<google::protobuf::string>(amount.toString()));
    request.mutable_from()->set_address(from_address.toString());
    request.mutable_fee()->set_value(static_cast<google::protobuf::uint64>(gas));
    request.set_init(init);
    request.set_contract_code(contract_code);
    request.mutable_creation_time()->set_since_epoch(transaction_time.getSecondsSinceEpoch());
    request.mutable_signature()->set_raw(signature.toBase64());

    // call remote host
    likelib::TransactionCreateContractResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->create_contract(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        auto converted_status = convert(reply.status());
        auto contract_address = lk::Address{ reply.contract_address().address() };
        auto gas_left = std::uint64_t{ reply.fee_left().value() };
        return { converted_status, contract_address, gas_left };
    }
    else {
        throw RpcError(status.error_message());
    }
}


std::tuple<OperationStatus, std::string, std::uint64_t> GrpcNodeClient::transaction_message_call(
  lk::Balance amount,
  const lk::Address& from_address,
  const lk::Address& to_address,
  const base::Time& transaction_time,
  std::uint64_t fee,
  const std::string& data,
  const lk::Sign& signature)
{
    // convert data for request
    likelib::TransactionMessageCallRequest request;
    request.mutable_value()->set_value(static_cast<google::protobuf::string>(amount.toString()));
    request.mutable_from()->set_address(from_address.toString());
    request.mutable_to()->set_address(to_address.toString());
    request.mutable_fee()->set_value(static_cast<google::protobuf::uint64>(fee));
    request.set_data(data);
    request.mutable_creation_time()->set_since_epoch(transaction_time.getSecondsSinceEpoch());
    request.set_signature(signature.toBase64());

    // call remote host
    likelib::TransactionMessageCallResponse reply;
    grpc::ClientContext context;
    grpc::Status status = _stub->message_call(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        auto converted_status = convert(reply.status());
        auto gas_left = std::uint64_t{ reply.fee_left().value() };
        auto message_from_contract = reply.contract_response();
        return { converted_status, message_from_contract, gas_left };
    }
    else {
        throw RpcError(status.error_message());
    }
}

} // namespace rpc
