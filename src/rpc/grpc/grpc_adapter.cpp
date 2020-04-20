#include "grpc_adapter.hpp"
#include "tools.hpp"


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
        lk::Address address{ request->address_at_base_58() };
        auto account = _service->getAccount(address);
        response->mutable_address()->set_address_at_base_58(request->address_at_base_58());
        response->mutable_balance()->set_value(account.balance);
        response->set_nonce(account.nonce);
        for (auto& tx_hash : account.transactions_hashes) {
            ::likelib::Hash hs;
            hs.set_bytes_base_64(base::base64Encode(tx_hash.getBytes()));
            response->mutable_hashes()->Add(std::move(hs));
        }
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}

::grpc::Status Adapter::get_node_info(::grpc::ServerContext* context,
                                      const ::likelib::None* request,
                                      ::likelib::NodeInfo* response)
{
    LOG_DEBUG << "received RPC get_node_info method call from " << context->peer();
    try {
        auto info = _service->getNodeInfo();
        response->mutable_top_block_hash()->set_bytes_base_64(base::base64Encode(info.top_block_hash.getBytes()));
        response->set_interface_version(info.api_version);
        response->set_peers_number(info.peers_number);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}

::grpc::Status Adapter::get_block(::grpc::ServerContext* context,
                                  const ::likelib::Hash* request,
                                  ::likelib::Block* response)
{
    LOG_DEBUG << "received RPC get_block method call from " << context->peer();
    try {
        base::Sha256 block_hash{ base::base64Decode(request->bytes_base_64()) };
        auto block = _service->getBlock(block_hash);
        response->set_depth(block.getDepth());
        response->set_nonce(block.getNonce());
        response->mutable_previous_block_hash()->set_bytes_base_64(
          base::base64Encode(block.getPrevBlockHash().getBytes()));
        response->mutable_coinbase()->set_address_at_base_58(base::base64Encode(block.getCoinbase().getBytes()));
        response->mutable_timestamp()->set_since_epoch(block.getTimestamp().getSecondsSinceEpoch());

        for (const auto& tx : block.getTransactions()) {
            likelib::Transaction tv;
            serializeTransaction(tx, tv);
            response->mutable_transactions()->Add(std::move(tv));
        }
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
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
        serializeTransaction(tx, *response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
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
        lk::TransactionBuilder builder;
        builder.setAmount(request->value().value());

        builder.setData(base::base64Decode(request->data().bytes_base_64()));

        builder.setFee(request->fee().value());

        lk::Address from_address{ request->from().address_at_base_58() };
        builder.setFrom(from_address);

        lk::Address to_address{ request->to().address_at_base_58() };
        builder.setTo(to_address);

        builder.setTimestamp(base::Time{ static_cast<uint_least32_t>(request->creation_time().since_epoch()) });

        builder.setSign(lk::Sign::fromBase64(request->signature().signature_bytes_at_base_64()));

        if (to_address.isNull()) {
            builder.setType(lk::Transaction::Type::CONTRACT_CREATION);
        }
        else {
            builder.setType(lk::Transaction::Type::MESSAGE_CALL);
        }

        auto tx = builder.build();

        auto result = _service->pushTransaction(tx);

        serializeTransactionStatus(result, *response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
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
        base::Sha256 transaction_hash{ base::base64Decode(request->bytes_base_64()) };
        auto result = _service->getTransactionResult(transaction_hash);
        serializeTransactionStatus(result, *response);
    }
    catch (const base::Error& e) {
        LOG_ERROR << e.what();
        return ::grpc::Status::CANCELLED;
    }
    return ::grpc::Status::OK;
}

} // namespace rpc
