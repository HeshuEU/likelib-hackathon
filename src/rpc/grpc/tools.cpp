#include "tools.hpp"

#include "rpc/error.hpp"

namespace rpc::grpc
{

likelib::AccountInfo_Type serializeAccountType(lk::AccountType type)
{
    switch (type) {
        case lk::AccountType::CLIENT:
            return likelib::AccountInfo_Type_CLIENT;
        case lk::AccountType::CONTRACT:
            return likelib::AccountInfo_Type_CONTRACT;
        default:
            RAISE_ERROR(base::InvalidArgument, "Invalid type");
    }
}


lk::AccountType deserializeAccountType(likelib::AccountInfo_Type type)
{
    switch (type) {
        case likelib::AccountInfo::Type::AccountInfo_Type_CLIENT:
            return lk::AccountType::CLIENT;
        case likelib::AccountInfo::Type::AccountInfo_Type_CONTRACT:
            return lk::AccountType::CONTRACT;
        default:
            RAISE_ERROR(base::InvalidArgument, "Invalid type");
    }
}


likelib::TransactionStatus_StatusCode serializeTransactionStatusCode(lk::TransactionStatus::StatusCode status_code)
{
    switch (status_code) {
        case lk::TransactionStatus::StatusCode::Success:
            return ::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_Success;
        case lk::TransactionStatus::StatusCode::Failed:
            return ::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_Failed;
        case lk::TransactionStatus::StatusCode::Revert:
            return ::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_Revert;
        case lk::TransactionStatus::StatusCode::BadQueryForm:
            return ::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_BadQueryForm;
        case lk::TransactionStatus::StatusCode::BadSign:
            return ::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_BadSign;
        case lk::TransactionStatus::StatusCode::Pending:
            return ::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_Pending;
        case lk::TransactionStatus::StatusCode::NotEnoughBalance:
            return ::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_NotEnoughBalance;
        default:
            RAISE_ERROR(base::InvalidArgument, "Invalid type");
    }
}


lk::TransactionStatus::StatusCode deserializeTransactionStatusCode(likelib::TransactionStatus_StatusCode status_code)
{
    switch (status_code) {
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_Success:
            return lk::TransactionStatus::StatusCode::Success;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_Failed:
            return lk::TransactionStatus::StatusCode::Failed;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_Revert:
            return lk::TransactionStatus::StatusCode::Revert;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_BadQueryForm:
            return lk::TransactionStatus::StatusCode::BadQueryForm;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_BadSign:
            return lk::TransactionStatus::StatusCode::BadSign;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_Pending:
            return lk::TransactionStatus::StatusCode::Pending;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_NotEnoughBalance:
            return lk::TransactionStatus::StatusCode::NotEnoughBalance;
        default:
            RAISE_ERROR(base::InvalidArgument, "Invalid type");
    }
}


likelib::TransactionStatus_ActionType serializeTransactionActionType(lk::TransactionStatus::ActionType action_type)
{
    switch (action_type) {
        case lk::TransactionStatus::ActionType::ContractCreation:
            return likelib::TransactionStatus_ActionType_ContractCreation;
        case lk::TransactionStatus::ActionType::ContractCall:
            return likelib::TransactionStatus_ActionType_ContractCall;
        case lk::TransactionStatus::ActionType::Transfer:
            return likelib::TransactionStatus_ActionType_Transfer;
        case lk::TransactionStatus::ActionType::None:
            return likelib::TransactionStatus_ActionType_None;
        default:
            RAISE_ERROR(base::InvalidArgument, "Invalid type");
    }
}


lk::TransactionStatus::ActionType deserializeTransactionActionType(likelib::TransactionStatus_ActionType action_type)
{
    switch (action_type) {
        case likelib::TransactionStatus_ActionType_ContractCall:
            return lk::TransactionStatus::ActionType::ContractCall;
        case likelib::TransactionStatus_ActionType_ContractCreation:
            return lk::TransactionStatus::ActionType::ContractCreation;
        case likelib::TransactionStatus_ActionType_Transfer:
            return lk::TransactionStatus::ActionType::Transfer;
        case likelib::TransactionStatus_ActionType_None:
            return lk::TransactionStatus::ActionType::None;
        default:
            RAISE_ERROR(base::InvalidArgument, "Invalid type");
    }
}


void serializeAddress(const lk::Address& from, likelib::Address* to)
{
    to->set_address_at_base_58(base::base58Encode(from.getBytes()));
}


lk::Address deserializeAddress(const likelib::Address* const address)
{
    return lk::Address{ base::base58Decode(address->address_at_base_58()) };
}


void serializeAccountInfo(const lk::AccountInfo& from, likelib::AccountInfo* to)
{
    to->set_type(serializeAccountType(from.type));
    serializeAddress(from.address, to->mutable_address());
    to->mutable_balance()->set_value(from.balance.str());
    to->set_nonce(from.nonce);
    for (const auto& tx_hash : from.transactions_hashes) {
        serializeHash(tx_hash, to->mutable_hashes()->Add());
    }
}


lk::AccountInfo deserializeAccountInfo(const likelib::AccountInfo* const info)
{
    auto balance = info->balance().value();
    auto nonce = info->nonce();
    std::vector<base::Sha256> hashes;
    for (const auto& hs : info->hashes()) {
        hashes.emplace_back(deserializeHash(&hs));
    }
    lk::AccountType type = deserializeAccountType(info->type());
    lk::Address address = deserializeAddress(&(info->address()));
    return { type, address, lk::Balance{ balance }, nonce, std::move(hashes) };
}


void serializeHash(const base::Sha256& from, likelib::Hash* to)
{
    to->set_bytes_base_64(base::base64Encode(from.getBytes()));
}


base::Sha256 deserializeHash(const ::likelib::Hash* const hash)
{
    return base::Sha256{ base::base64Decode(hash->bytes_base_64()) };
}


void serializeInfo(const Info& from, likelib::NodeInfo* to)
{
    serializeHash(from.top_block_hash, to->mutable_top_block_hash());
    to->set_top_block_number(from.top_block_number);
    to->set_interface_version(from.api_version);
}


Info deserializeInfo(const likelib::NodeInfo* const info)
{
    auto top_block_hash = deserializeHash(&(info->top_block_hash()));
    return { top_block_hash, info->top_block_number(), info->interface_version() };
}


void serializeNumber(uint64_t from, likelib::Number* to)
{
    to->set_number(from);
}


base::Bytes deserializeData(const likelib::Data* const data)
{
    return base::base64Decode(data->bytes_base_64());
}


void serializeTransaction(const lk::Transaction& from, likelib::Transaction* to)
{
    serializeAddress(from.getFrom(), to->mutable_from());
    serializeAddress(from.getTo(), to->mutable_to());
    to->mutable_value()->set_value(from.getAmount().str());
    to->set_fee(from.getFee());
    to->mutable_creation_time()->set_seconds_since_epoch(from.getTimestamp().getSeconds());
    to->set_data(base::base64Encode(from.getData()));
    to->mutable_signature()->set_signature_bytes_at_base_64(base::base64Encode(from.getSign()));
}


lk::Transaction deserializeTransaction(const ::likelib::Transaction* const tx)
{
    lk::TransactionBuilder txb;
    txb.setFrom(deserializeAddress(&tx->from()));
    lk::Address to_address = deserializeAddress(&tx->to());
    txb.setTo(to_address);
    txb.setAmount(lk::Balance{ tx->value().value() });
    txb.setFee(tx->fee());
    txb.setTimestamp(base::Time(tx->creation_time().seconds_since_epoch()));
    txb.setData(base::base64Decode(tx->data()));
    txb.setSign(lk::Sign(base::base64Decode(tx->signature().signature_bytes_at_base_64())));
    return std::move(txb).build();
}


void serializeBlock(const lk::ImmutableBlock& from, likelib::Block* to)
{
    to->set_depth(from.getDepth());
    to->set_nonce(from.getNonce());
    serializeHash(from.getPrevBlockHash(), to->mutable_previous_block_hash());
    serializeAddress(from.getCoinbase(), to->mutable_coinbase());
    to->mutable_timestamp()->set_seconds_since_epoch(from.getTimestamp().getSeconds());
    for (const auto& tx : from.getTransactions()) {
        serializeTransaction(tx, to->mutable_transactions()->Add());
    }
}


lk::ImmutableBlock deserializeBlock(const likelib::Block* const block_to_deserialization)
{
    lk::BlockDepth depth{ block_to_deserialization->depth() };
    base::Sha256 prev_block_hash = deserializeHash(&(block_to_deserialization->previous_block_hash()));
    base::Time timestamp{ block_to_deserialization->timestamp().seconds_since_epoch() };
    lk::NonceInt nonce{ block_to_deserialization->nonce() };
    lk::Address coinbase = deserializeAddress(&(block_to_deserialization->coinbase()));
    lk::TransactionsSet txset;
    for (const auto& txv : block_to_deserialization->transactions()) {
        txset.add(deserializeTransaction(&txv));
    }

    lk::BlockBuilder b;
    b.setDepth(depth);
    b.setNonce(nonce);
    b.setPrevBlockHash(std::move(prev_block_hash));
    b.setTimestamp(std::move(timestamp));
    b.setCoinbase(std::move(coinbase));
    b.setTransactionsSet(std::move(txset));
    return std::move(b).buildImmutable();
}


void serializeTransactionStatus(const lk::TransactionStatus& from, likelib::TransactionStatus* to)
{
    to->set_fee_left(from.getFeeLeft());
    to->set_message(from.getMessage());
    to->set_status(serializeTransactionStatusCode(from.getStatus()));
    to->set_type(serializeTransactionActionType(from.getType()));
}


lk::TransactionStatus deserializeTransactionStatus(const likelib::TransactionStatus* const status)
{
    lk::TransactionStatus::StatusCode status_code = deserializeTransactionStatusCode(status->status());
    lk::TransactionStatus::ActionType action_type = deserializeTransactionActionType(status->type());
    return lk::TransactionStatus{ status_code, action_type, status->fee_left(), status->message() };
}

}