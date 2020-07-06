#include "grpc_client.hpp"

#include "rpc/grpc/tools.hpp"

#include "rpc/error.hpp"


namespace rpc::grpc
{

NodeClient::NodeClient(const std::string& connect_address)
{
    auto channel_credentials = ::grpc::InsecureChannelCredentials();
    _stub =
      std::make_unique<likelib::NodePublicInterface::Stub>(::grpc::CreateChannel(connect_address, channel_credentials));
}


lk::AccountInfo NodeClient::getAccountInfo(const lk::Address& address)
{
    // convert data for request
    likelib::Address request;
    try {
        serializeAddress(address, &request);
    }
    catch (const base::Error& er) {
        RAISE_ERROR(RpcError, std::string("serialization error: ") + er.what());
    }
    // call remote host
    likelib::AccountInfo reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_account(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        try {
            return deserializeAccountInfo(&reply);
        }
        catch (const base::Error& er) {
            RAISE_ERROR(RpcError, std::string("deserialization error: ") + er.what());
        }
    }
    else {
        RAISE_ERROR(RpcError, status.error_message());
    }
}


Info NodeClient::getNodeInfo()
{
    // convert data for request
    likelib::None request;

    // call remote host
    likelib::NodeInfo reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_node_info(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        try {
            return deserializeInfo(&reply);
        }
        catch (const base::Error& er) {
            RAISE_ERROR(RpcError, std::string("deserialization error: ") + er.what());
        }
    }
    else {
        RAISE_ERROR(RpcError, status.error_message());
    }
}


lk::ImmutableBlock NodeClient::getBlock(const base::Sha256& block_hash)
{
    // convert data for request
    likelib::Hash request;
    try {
        serializeHash(block_hash, &request);
    }
    catch (const base::Error& er) {
        RAISE_ERROR(RpcError, std::string("serialization error: ") + er.what());
    }

    // call remote host
    likelib::Block reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_block_by_hash(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        try {
            return deserializeBlock(&reply);
        }
        catch (const base::Error& er) {
            RAISE_ERROR(RpcError, std::string("deserialization error: ") + er.what());
        }
    }
    else {
        RAISE_ERROR(RpcError, status.error_message());
    }
}


lk::ImmutableBlock NodeClient::getBlock(uint64_t block_number)
{
    // convert data for request
    likelib::Number request;
    try {
        serializeNumber(block_number, &request);
    }
    catch (const base::Error& er) {
        RAISE_ERROR(RpcError, std::string("serialization error: ") + er.what());
    }

    // call remote host
    likelib::Block reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_block_by_number(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        try {
            return deserializeBlock(&reply);
        }
        catch (const base::Error& er) {
            RAISE_ERROR(RpcError, std::string("deserialization error: ") + er.what());
        }
    }
    else {
        RAISE_ERROR(RpcError, status.error_message());
    }
}


lk::Transaction NodeClient::getTransaction(const base::Sha256& transaction_hash)
{
    // convert data for request
    likelib::Hash request;
    try {
        serializeHash(transaction_hash, &request);
    }
    catch (const base::Error& er) {
        RAISE_ERROR(RpcError, std::string("serialization error: ") + er.what());
    }

    // call remote host
    likelib::Transaction reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_transaction(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        try {
            return deserializeTransaction(&reply);
        }
        catch (const base::Error& er) {
            RAISE_ERROR(RpcError, std::string("deserialization error: ") + er.what());
        }
    }
    else {
        RAISE_ERROR(RpcError, status.error_message());
    }
}


lk::TransactionStatus NodeClient::pushTransaction(const lk::Transaction& transaction)
{
    // convert data for request
    likelib::Transaction request;
    try {
        serializeTransaction(transaction, &request);
    }
    catch (const base::Error& er) {
        RAISE_ERROR(RpcError, std::string("serialization error: ") + er.what());
    }

    // call remote host
    likelib::TransactionStatus reply;
    ::grpc::ClientContext context;
    auto status = _stub->push_transaction(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        try {
            return deserializeTransactionStatus(&reply);
        }
        catch (const base::Error& er) {
            RAISE_ERROR(RpcError, std::string("deserialization error: ") + er.what());
        }
    }
    else {
        RAISE_ERROR(RpcError, status.error_message());
    }
}


lk::TransactionStatus NodeClient::getTransactionStatus(const base::Sha256& transaction_hash)
{
    // convert data for request
    likelib::Hash request;
    try {
        serializeHash(transaction_hash, &request);
    }
    catch (const base::Error& er) {
        RAISE_ERROR(RpcError, std::string("serialization error: ") + er.what());
    }

    // call remote host
    likelib::TransactionStatus reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_transaction_result(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        try {
            return deserializeTransactionStatus(&reply);
        }
        catch (const base::Error& er) {
            RAISE_ERROR(RpcError, std::string("deserialization error: ") + er.what());
        }
    }
    else {
        RAISE_ERROR(RpcError, status.error_message());
    }
}

} // namespace rpc::grpc
