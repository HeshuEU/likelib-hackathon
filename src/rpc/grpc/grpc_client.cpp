#include "grpc_client.hpp"
#include "tools.hpp"

#include <rpc/error.hpp>


namespace rpc::grpc
{


NodeClient::NodeClient(const std::string& connect_address)
{
    auto channel_credentials = ::grpc::InsecureChannelCredentials();
    _stub =
      std::make_unique<likelib::NodePublicInterface::Stub>(::grpc::CreateChannel(connect_address, channel_credentials));
}


lk::AccountInfo NodeClient::getAccount(const lk::Address& address)
{
    // convert data for request
    likelib::Address request;
    request.set_address_at_base_58(base::base58Encode(address.getBytes()));

    // call remote host
    likelib::AccountInfo reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_account(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        auto balance = reply.balance().value();
        auto nonce = reply.nonce();
        std::vector<base::Sha256> hashes;
        for (auto& hs : reply.hashes()) {
            base::Sha256 hash{ base::base64Decode(hs.bytes_base_64()) };
            hashes.emplace_back(std::move(hash));
        }

        return { address, balance, nonce, std::move(hashes) };
    }
    else {
        throw RpcError(status.error_message());
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
        auto top_block_hash = base::Sha256{ base::base64Decode(reply.top_block_hash().bytes_base_64()) };
        return { std::move(top_block_hash), reply.interface_version(), reply.peers_number() };
    }
    else {
        throw RpcError(status.error_message());
    }
}


lk::Block NodeClient::getBlock(const base::Sha256& block_hash)
{
    // convert data for request
    likelib::Hash request;
    request.set_bytes_base_64(base::base64Encode(block_hash.getBytes()));

    // call remote host
    likelib::Block reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_block(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        lk::BlockDepth depth{ reply.depth() };
        base::Sha256 prev_block_hash{ base::base64Decode(reply.previous_block_hash().bytes_base_64()) };
        base::Time timestamp(reply.timestamp().since_epoch());
        lk::NonceInt nonce{ reply.nonce() };
        lk::Address coinbase{ base::base58Decode(reply.coinbase().address_at_base_58()) };

        lk::TransactionsSet txset;
        for (const auto& txv : reply.transactions()) {
            txset.add(deserializeTransaction(txv));
        }

        lk::Block blk{ depth, std::move(prev_block_hash), timestamp, std::move(coinbase), std::move(txset) };
        blk.setNonce(nonce);
        return blk;
    }
    else {
        throw RpcError(status.error_message());
    }
}


lk::Transaction NodeClient::getTransaction(const base::Sha256& transaction_hash)
{
    // convert data for request
    likelib::Hash request;
    request.set_bytes_base_64(base::base64Encode(transaction_hash.getBytes()));

    // call remote host
    likelib::Transaction reply;
    ::grpc::ClientContext context;
    auto status = _stub->get_transaction(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        return deserializeTransaction(reply);
    }
    else {
        throw RpcError(status.error_message());
    }
}


TransactionStatus NodeClient::pushTransaction(const lk::Transaction& transaction)
{
    // convert data for request
    likelib::Transaction request;
    serializeTransaction(transaction, request);

    // call remote host
    likelib::TransactionStatus reply;
    ::grpc::ClientContext context;

    auto status = _stub->push_transaction(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        return deserializeTransactionStatus(reply);
    }
    else {
        throw RpcError(status.error_message());
    }
}


TransactionStatus NodeClient::getTransactionResult(const base::Sha256& transaction_hash)
{
    // convert data for request
    likelib::Hash request;
    request.set_bytes_base_64(base::base64Encode(transaction_hash.getBytes()));

    // call remote host
    likelib::TransactionStatus reply;
    ::grpc::ClientContext context;

    auto status = _stub->get_transaction_result(&context, request, &reply);

    // return value if ok
    if (status.ok()) {
        return deserializeTransactionStatus(reply);
    }
    else {
        throw RpcError(status.error_message());
    }
}


} // namespace rpc
