#include "tools.hpp"

namespace rpc::grpc
{

void serializeTransaction(const lk::Transaction& from, likelib::Transaction& to)
{
    to.mutable_from()->set_address_at_base_58(from.getFrom().toString());
    to.mutable_to()->set_address_at_base_58(from.getTo().toString());
    to.mutable_value()->set_value(from.getAmount());
    to.mutable_fee()->set_value(from.getFee());
    to.mutable_creation_time()->set_since_epoch(from.getTimestamp().getSecondsSinceEpoch());
    to.mutable_data()->set_bytes_base_64(base::base64Encode(from.getData()));
    to.mutable_signature()->set_signature_bytes_at_base_64(from.getSign().toBase64());
}

lk::Transaction&& deserializeTransaction(const ::likelib::Transaction& serialized_tx)
{
    lk::TransactionBuilder txb;
    txb.setFrom(lk::Address(base::base58Decode(serialized_tx.from().address_at_base_58())));
    lk::Address to_address{ base::base58Decode(serialized_tx.to().address_at_base_58()) };
    txb.setTo(to_address);
    txb.setAmount(serialized_tx.value().value());
    txb.setFee(serialized_tx.fee().value());
    txb.setTimestamp(base::Time(serialized_tx.creation_time().since_epoch()));
    txb.setData(base::base64Decode(serialized_tx.data().bytes_base_64()));
    txb.setSign(lk::Sign::fromBase64(serialized_tx.signature().signature_bytes_at_base_64()));

    txb.setType(to_address == lk::Address::null() ? lk::Transaction::Type::CONTRACT_CREATION :
                                                    lk::Transaction::Type::MESSAGE_CALL);
    return std::move(std::move(txb).build());
}


void serializeTransactionStatus(const TransactionStatus& from, likelib::TransactionStatus& to)
{
    to.mutable_gas_left()->set_value(from.getGasLeft());
    to.set_message(from.getMessage());
    switch (from.getStatus()) {
        case rpc::TransactionStatus::StatusCode::Success:
            to.set_status(::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_Success);
            break;
        case rpc::TransactionStatus::StatusCode::Failed:
            to.set_status(::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_Failed);
            break;
        case rpc::TransactionStatus::StatusCode::Revert:
            to.set_status(::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_Revert);
            break;
        case rpc::TransactionStatus::StatusCode::Rejected:
            to.set_status(::likelib::TransactionStatus::StatusCode::TransactionStatus_StatusCode_Rejected);
            break;
    }
}


TransactionStatus deserializeTransactionStatus(const likelib::TransactionStatus& status)
{
    TransactionStatus::StatusCode status_code = rpc::TransactionStatus::StatusCode::Failed;
    switch (status.status()) {
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_Success:
            status_code = rpc::TransactionStatus::StatusCode::Success;
            break;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_Failed:
            status_code = rpc::TransactionStatus::StatusCode::Failed;
            break;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_Revert:
            status_code = rpc::TransactionStatus::StatusCode::Revert;
            break;
        case likelib::TransactionStatus_StatusCode::TransactionStatus_StatusCode_Rejected:
            status_code = rpc::TransactionStatus::StatusCode::Rejected;
            break;
    }
    return { status_code, status.message(), status.gas_left() };
}


}