#include "tools.hpp"

namespace rpc::http
{

web::json::value serializeAccountType(lk::AccountType type)
{
    switch (type) {
        case lk::AccountType::CONTRACT:
            return web::json::value::string("Contract");
        case lk::AccountType::CLIENT:
            return web::json::value::string("Client");
        default:
            RAISE_ERROR(base::InvalidArgument, "Invalid type");
    }
}


std::optional<lk::AccountType> deserializeAccountType(const std::string& type)
{
    if (type == "Client") {
        return lk::AccountType::CLIENT;
    }
    else if (type == "Contract") {
        return lk::AccountType::CONTRACT;
    }
    else {
        LOG_ERROR << "Failed to deserialize AccountType";
        return std::nullopt;
    }
}


web::json::value serializeTransactionStatusStatusCode(lk::TransactionStatus::StatusCode status_code)
{
    return web::json::value::number(static_cast<uint32_t>(status_code));
}


std::optional<lk::TransactionStatus::StatusCode> deserializeTransactionStatusStatusCode(std::uint32_t type)
{
    switch (type) {
        case 0:
            return lk::TransactionStatus::StatusCode::Success;
        case 1:
            return lk::TransactionStatus::StatusCode::Pending;
        case 2:
            return lk::TransactionStatus::StatusCode::BadQueryForm;
        case 3:
            return lk::TransactionStatus::StatusCode::BadSign;
        case 4:
            return lk::TransactionStatus::StatusCode::NotEnoughBalance;
        case 5:
            return lk::TransactionStatus::StatusCode::Revert;
        case 6:
            return lk::TransactionStatus::StatusCode::Failed;
        default:
            LOG_ERROR << "Failed to deserialize StatusCode";
            return std::nullopt;
    }
}


web::json::value serializeTransactionStatusActionType(lk::TransactionStatus::ActionType action_type)
{
    return web::json::value::number(static_cast<uint32_t>(action_type));
}


std::optional<lk::TransactionStatus::ActionType> deserializeTransactionStatusActionType(std::uint32_t type)
{
    switch (type) {
        case 0:
            return lk::TransactionStatus::ActionType::None;
        case 1:
            return lk::TransactionStatus::ActionType::Transfer;
        case 2:
            return lk::TransactionStatus::ActionType::ContractCall;
        case 3:
            return lk::TransactionStatus::ActionType::ContractCreation;
        default:
            LOG_ERROR << "Failed to deserialize ActionType";
            return std::nullopt;
    }
}


web::json::value serializeBalance(const lk::Balance& balance)
{
    return web::json::value::string(balance.str());
}


std::optional<lk::Balance> deserializeBalance(const std::string& type)
{
    try {
        return lk::Balance{ type };
    }
    catch (const base::Error& e) {
        LOG_ERROR << "Failed to deserialize Balance";
        return std::nullopt;
    }
}


web::json::value serializeFee(std::uint64_t balance)
{
    return web::json::value::string(std::to_string(balance));
}


std::optional<std::uint64_t> deserializeFee(const std::string& type)
{
    char* end = nullptr;
    auto balance = static_cast<uint64_t>(std::strtoll(type.c_str(), &end, 10));
    if (end == nullptr) {
        LOG_ERROR << "Failed to deserialize fee";
        return std::nullopt;
    }
    return balance;
}


web::json::value serializeHash(const base::Sha256& hash)
{
    return web::json::value::string(base::base64Encode(hash.getBytes()));
}


std::optional<base::Sha256> deserializeHash(const std::string& hash_str)
{
    base::Bytes decoded_bytes;
    try {
        decoded_bytes = base::base64Decode(hash_str);
    }
    catch (const base::Error& e) {
        LOG_ERROR << "Failed to deserialize hash data";
        return std::nullopt;
    }

    try {
        return base::Sha256{ decoded_bytes };
    }
    catch (const base::Error& e) {
        LOG_ERROR << "Failed to deserialize hash";
        return std::nullopt;
    }
}


web::json::value serializeAddress(const lk::Address& address)
{
    return web::json::value::string(base::base58Encode(address.getBytes()));
}


std::optional<lk::Address> deserializeAddress(const std::string& address_str)
{
    base::Bytes decoded_bytes;
    try {
        decoded_bytes = base::base58Decode(address_str);
    }
    catch (const base::Error& e) {
        LOG_ERROR << "Failed to deserialize address data";
        return std::nullopt;
    }

    try {
        return lk::Address{ decoded_bytes };
    }
    catch (const base::Error& e) {
        LOG_ERROR << "Failed to deserialize address";
        return std::nullopt;
    }
}


web::json::value serializeBytes(const base::Bytes& data)
{
    return web::json::value::string(base::base64Encode(data));
}


std::optional<base::Bytes> deserializeBytes(const std::string& data)
{
    try {
        return base::base64Decode(data);
    }
    catch (const base::Error& e) {
        LOG_ERROR << "Failed to deserialize data bytes";
        return std::nullopt;
    }
}


web::json::value serializeSign(const lk::Sign& sign)
{
    return web::json::value::string(base::base64Encode(sign.toBytes()));
}


std::optional<lk::Sign> deserializeSign(const std::string& data)
{
    auto sign_data = deserializeBytes(data);
    if (!sign_data) {
        LOG_ERROR << "base sign format";
        return std::nullopt;
    }
    return lk::Sign(sign_data.value());
}


web::json::value serializeAccountInfo(const lk::AccountInfo& account_info)
{
    web::json::value result;
    result["address"] = rpc::http::serializeAddress(account_info.address);
    result["balance"] = serializeBalance(account_info.balance);
    result["nonce"] = web::json::value::number(account_info.nonce);
    result["type"] = serializeAccountType(account_info.type);
    std::vector<web::json::value> txs_hashes;
    for (const auto& tx_hash : account_info.transactions_hashes) {
        txs_hashes.emplace_back(serializeHash(tx_hash));
    }
    result["transaction_hashes"] = web::json::value::array(txs_hashes);
    return result;
}


std::optional<lk::AccountInfo> deserializeAccountInfo(const web::json::value& input)
{
    try {
        std::optional<lk::AccountType> type;
        if (input.has_string_field("type")) {
            type = deserializeAccountType(input.at("type").as_string());
        }
        else {
            LOG_ERROR << "type field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Balance> balance;
        if (input.has_string_field("balance")) {
            balance = deserializeBalance(input.at("balance").as_string());
        }
        else {
            LOG_ERROR << "balance field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> nonce;
        if (input.has_number_field("nonce")) {
            nonce = input.at("nonce").as_number().to_uint64();
        }
        else {
            LOG_ERROR << "nonce field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Address> address;
        if (input.has_string_field("address")) {
            address = deserializeAddress(input.at("address").as_string());
        }
        else {
            LOG_ERROR << "address field is not exists";
            return std::nullopt;
        }

        std::vector<base::Sha256> transactions_hashes;
        if (input.has_array_field("transaction_hashes")) {
            for (const auto& res_tx_hash : input.at("transaction_hashes").as_array()) {
                if (res_tx_hash.is_string()) {
                    auto hash_opt = deserializeHash(res_tx_hash.as_string());
                    if (hash_opt) {
                        transactions_hashes.emplace_back(*hash_opt);
                    }
                    else {
                        LOG_ERROR << "error at hash deserialization";
                        return std::nullopt;
                    }
                }
                else {
                    LOG_ERROR << "bad hash format";
                    return std::nullopt;
                }
            }
        }
        else {
            LOG_ERROR << "transaction_hashes field is not exists";
            return std::nullopt;
        }

        if (!type) {
            LOG_ERROR << "error at type deserialization";
            return std::nullopt;
        }

        if (!address) {
            LOG_ERROR << "error at address deserialization";
            return std::nullopt;
        }

        if (!balance) {
            LOG_ERROR << "error at balance deserialization";
            return std::nullopt;
        }

        if (!nonce) {
            LOG_ERROR << "error at nonce deserialization";
            return std::nullopt;
        }

        return lk::AccountInfo{ type.value(), address.value(), balance.value(), nonce.value(), transactions_hashes };
    }
    catch (const base::Error& e) {
        LOG_ERROR << "Failed to deserialize Account Info";
        return std::nullopt;
    }
}


web::json::value serializeInfo(const Info& info)
{
    web::json::value result;
    result["top_block_hash"] = serializeHash(info.top_block_hash);
    result["top_block_number"] = web::json::value::number(info.top_block_number);
    result["api_version"] = web::json::value::number(info.api_version);
    return result;
}


std::optional<Info> deserializeInfo(const web::json::value& input)
{
    try {
        std::optional<base::Sha256> top_block_hash;
        if (input.has_string_field("top_block_hash")) {
            top_block_hash = deserializeHash(input.at("top_block_hash").as_string());
        }
        else {
            LOG_ERROR << "top_block_hash field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> top_block_number;
        if (input.has_number_field("top_block_number")) {
            top_block_number = input.at("top_block_number").as_number().to_uint64();
        }
        else {
            LOG_ERROR << "top_block_number field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint32_t> api_version;
        if (input.has_number_field("api_version")) {
            api_version = input.at("api_version").as_number().to_uint32();
        }
        else {
            LOG_ERROR << "api_version field is not exists";
            return std::nullopt;
        }

        if (!top_block_hash) {
            LOG_ERROR << "error at top_block_hash deserialization";
            return std::nullopt;
        }

        if (!top_block_number) {
            LOG_ERROR << "error at top_block_number deserialization";
            return std::nullopt;
        }

        if (!api_version) {
            LOG_ERROR << "error at api_version deserialization";
            return std::nullopt;
        }
        return Info{ top_block_hash.value(), top_block_number.value(), api_version.value() };
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Failed to deserialize Info";
        return std::nullopt;
    }
}


web::json::value serializeTransaction(const lk::Transaction& input)
{
    web::json::value result;
    result["from"] = serializeAddress(input.getFrom());
    result["to"] = serializeAddress(input.getTo());
    result["amount"] = serializeBalance(input.getAmount());
    result["fee"] = serializeFee(input.getFee());
    result["timestamp"] = web::json::value::number(input.getTimestamp().getSeconds());
    result["data"] = serializeBytes(input.getData());
    result["sign"] = serializeSign(input.getSign());
    return result;
}


std::optional<lk::Transaction> deserializeTransaction(const web::json::value& input)
{
    try {
        std::optional<lk::Balance> amount;
        if (input.has_string_field("amount")) {
            amount = deserializeBalance(input.at("amount").as_string());
        }
        else {
            LOG_ERROR << "amount field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> fee;
        if (input.has_string_field("fee")) {
            fee = deserializeFee(input.at("fee").as_string());
        }
        else {
            LOG_ERROR << "fee field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Address> from;
        if (input.has_string_field("from")) {
            from = deserializeAddress(input.at("from").as_string());
        }
        else {
            LOG_ERROR << "from field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Address> to;
        if (input.has_string_field("to")) {
            to = deserializeAddress(input.at("to").as_string());
        }
        else {
            LOG_ERROR << "to field is not exists";
            return std::nullopt;
        }
        std::optional<base::Time> timestamp;
        if (input.has_number_field("timestamp")) {
            timestamp = base::Time(input.at("timestamp").as_number().to_uint32());
        }
        else {
            LOG_ERROR << "timestamp field is not exists";
            return std::nullopt;
        }
        std::optional<base::Bytes> data;
        if (input.has_string_field("data")) {
            data = deserializeBytes(input.at("data").as_string());
        }
        else {
            LOG_ERROR << "data field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Sign> sign;
        if (input.has_string_field("sign")) {
            sign = deserializeSign(input.at("sign").as_string());
        }
        else {
            LOG_ERROR << "sign field is not exists";
            return std::nullopt;
        }
        if (!from) {
            LOG_ERROR << "error at from deserialization";
            return std::nullopt;
        }
        if (!to) {
            LOG_ERROR << "error at to deserialization";
            return std::nullopt;
        }
        if (!fee) {
            LOG_ERROR << "error at fee deserialization";
            return std::nullopt;
        }
        if (!timestamp) {
            LOG_ERROR << "error at timestamp deserialization";
            return std::nullopt;
        }
        if (!data) {
            LOG_ERROR << "error at data deserialization";
            return std::nullopt;
        }
        if (!sign) {
            LOG_ERROR << "error at sign deserialization";
            return std::nullopt;
        }
        return lk::Transaction{ from.value(),      to.value(),   amount.value(), fee.value(),
                                timestamp.value(), data.value(), sign.value() };
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Failed to deserialize Transaction";
        return std::nullopt;
    }
}


web::json::value serializeBlock(const lk::ImmutableBlock& block)
{
    web::json::value result;
    result["depth"] = web::json::value::number(block.getDepth());
    result["nonce"] = web::json::value::number(block.getNonce());
    result["coinbase"] = serializeAddress(block.getCoinbase());
    result["previous_block_hash"] = serializeHash(block.getPrevBlockHash());
    result["timestamp"] = web::json::value::number(block.getTimestamp().getSeconds());

    std::vector<web::json::value> txs_values;
    for (auto& tx : block.getTransactions()) {
        txs_values.emplace_back(serializeTransaction(tx));
    }
    result["transactions"] = web::json::value::array(txs_values);
    return result;
}


std::optional<lk::ImmutableBlock> deserializeBlock(const web::json::value& input)
{
    try {
        std::optional<std::uint64_t> depth;
        if (input.has_number_field("depth")) {
            depth = input.at("depth").as_number().to_uint64();
        }
        else {
            LOG_ERROR << "depth field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> nonce;
        if (input.has_number_field("nonce")) {
            nonce = input.at("nonce").as_number().to_uint64();
        }
        else {
            LOG_ERROR << "nonce field is not exists";
            return std::nullopt;
        }
        std::optional<base::Time> timestamp;
        if (input.has_number_field("timestamp")) {
            timestamp = base::Time(input.at("timestamp").as_number().to_uint32());
        }
        else {
            LOG_ERROR << "timestamp field is not exists";
            return std::nullopt;
        }
        std::optional<base::Sha256> previous_block_hash;
        if (input.has_string_field("previous_block_hash")) {
            previous_block_hash = deserializeHash(input.at("previous_block_hash").as_string());
        }
        else {
            LOG_ERROR << "previous_block_hash field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Address> coinbase;
        if (input.has_string_field("coinbase")) {
            coinbase = deserializeAddress(input.at("coinbase").as_string());
        }
        else {
            LOG_ERROR << "coinbase field is not exists";
            return std::nullopt;
        }

        if (!depth) {
            LOG_ERROR << "error at depth deserialization";
            return std::nullopt;
        }
        if (!nonce) {
            LOG_ERROR << "error at nonce deserialization";
            return std::nullopt;
        }
        if (!timestamp) {
            LOG_ERROR << "error at timestamp deserialization";
            return std::nullopt;
        }
        if (!previous_block_hash) {
            LOG_ERROR << "error at previous_block_hash deserialization";
            return std::nullopt;
        }
        if (!coinbase) {
            LOG_ERROR << "error at coinbase deserialization";
            return std::nullopt;
        }

        lk::TransactionsSet txs;
        if (input.has_array_field("transactions")) {
            for (const auto& res_tx : input.at("transactions").as_array()) {
                if (res_tx.is_object()) {
                    auto tx_opt = deserializeTransaction(res_tx);
                    if (tx_opt) {
                        txs.add(*tx_opt);
                    }
                    else {
                        LOG_ERROR << "error at hash deserialization";
                        return std::nullopt;
                    }
                }
                else {
                    LOG_ERROR << "bad transaction format";
                    return std::nullopt;
                }
            }
        }
        else {
            LOG_ERROR << "transactions field is not exists";
            return std::nullopt;
        }

        lk::BlockBuilder b;
        b.setDepth(depth.value());
        b.setNonce(nonce.value());
        b.setPrevBlockHash(previous_block_hash.value());
        b.setTimestamp(timestamp.value());
        b.setCoinbase(coinbase.value());
        b.setTransactionsSet(std::move(txs));
        return std::move(b).buildImmutable();
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Failed to deserialize Block";
        return std::nullopt;
    }
}


web::json::value serializeTransactionStatus(const lk::TransactionStatus& status)
{
    web::json::value result;
    result["status_code"] = serializeTransactionStatusStatusCode(status.getStatus());
    result["action_type"] = serializeTransactionStatusActionType(status.getType());
    result["fee_left"] = serializeFee(status.getFeeLeft());
    result["message"] = web::json::value::string(status.getMessage());
    return result;
}


std::optional<lk::TransactionStatus> deserializeTransactionStatus(const web::json::value& input)
{
    try {
        std::optional<lk::TransactionStatus::StatusCode> status_code;
        if (input.has_number_field("status_code")) {
            status_code = deserializeTransactionStatusStatusCode(input.at("status_code").as_number().to_uint32());
        }
        else {
            LOG_ERROR << "status_code field is not exists";
            return std::nullopt;
        }
        std::optional<lk::TransactionStatus::ActionType> action_type;
        if (input.has_number_field("action_type")) {
            action_type = deserializeTransactionStatusActionType(input.at("action_type").as_number().to_uint32());
        }
        else {
            LOG_ERROR << "action_type field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> fee;
        if (input.has_string_field("fee_left")) {
            fee = deserializeFee(input.at("fee_left").as_string());
        }
        else {
            LOG_ERROR << "fee_left field is not exists";
            return std::nullopt;
        }
        std::optional<std::string> message;
        if (input.has_string_field("message")) {
            message = input.at("message").as_string();
        }
        else {
            LOG_ERROR << "message field is not exists";
            return std::nullopt;
        }

        if (!status_code) {
            LOG_ERROR << "error at status_code deserialization";
            return std::nullopt;
        }
        if (!action_type) {
            LOG_ERROR << "error at action_type deserialization";
            return std::nullopt;
        }
        if (!fee) {
            LOG_ERROR << "error at fee deserialization";
            return std::nullopt;
        }
        if (!message) {
            LOG_ERROR << "error at message deserialization";
            return std::nullopt;
        }
        return lk::TransactionStatus{ status_code.value(), action_type.value(), fee.value(), message.value() };
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Failed to deserialize TransactionStatus";
        return std::nullopt;
    }
}

}