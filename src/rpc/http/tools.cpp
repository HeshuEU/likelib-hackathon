#include "tools.hpp"

namespace rpc::http
{


web::json::value serializeAccountType(lk::AccountType type)
{
    switch (type) {
        case lk::AccountType::CONTRACT:
            return web::json::value::string("contract");
        case lk::AccountType::CLIENT:
            return web::json::value::string("client");
        default:
            RAISE_ERROR(base::InvalidArgument, "Invalid type");
    }
}


std::optional<lk::AccountType> deserializeAccountType(const std::string& type)
{
    if (type == "client") {
        return lk::AccountType::CLIENT;
    }
    else if (type == "contract") {
        return lk::AccountType::CONTRACT;
    }
    else {
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
            return lk::TransactionStatus::StatusCode::Rejected;
        case 2:
            return lk::TransactionStatus::StatusCode::Revert;
        case 3:
            return lk::TransactionStatus::StatusCode::Failed;
        default:
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
            return std::nullopt;
    }
}


web::json::value serializeBalance(const lk::Balance& balance)
{
    return web::json::value::string(std::to_string(balance));
}


std::optional<lk::Balance> deserializeBalance(const std::string& type)
{
    char* end = nullptr;
    auto balance = lk::Balance{ static_cast<uint64_t>(std::strtoll(type.c_str(), &end, 10)) };
    if (end == nullptr) {
        return std::nullopt;
    }
    return balance;
}


web::json::value serializeHash(const base::Sha256& hash)
{
    return web::json::value::string(base::base64Encode(hash.getBytes()));
}


std::optional<base::Sha256> deserializeHash(const std::string& type)
{
    try {
        return base::Sha256{ base::base64Decode(type) };
    }
    catch (const base::Error& e) {
        return std::nullopt;
    }
}


web::json::value serializeAddress(const lk::Address& address)
{
    return web::json::value::string(base::base58Encode(address.getBytes()));
}


std::optional<lk::Address> deserializeAddress(const std::string& type)
{
    try {
        return lk::Address{ base::base58Decode(type) };
    }
    catch (const base::Error& e) {
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
        return std::nullopt;
    }
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
    result["abi"] = web::json::value::string(account_info.serialized_abi);

    return result;
}


std::optional<lk::AccountInfo> deserializeAccountInfo(const web::json::value& input)
{
    try {
        auto type = deserializeAccountType(input.at("type").as_string());
        auto balance = deserializeBalance(input.at("balance").as_string());
        auto nonce = input.at("nonce").as_number().to_uint64();
        auto address = deserializeAddress(input.at("address").as_string());

        std::string serialized_abi;
        if (type == lk::AccountType::CONTRACT) {
            serialized_abi = input.at("abi").as_string();
        }

        std::vector<base::Sha256> transactions_hashes;
        for (const auto& res_tx_hash : input.at("transaction_hashes").as_array()) {
            transactions_hashes.emplace_back(deserializeHash(res_tx_hash.as_string()).value());
        }

        return lk::AccountInfo{ type.value(), address.value(),     balance.value(),
                                nonce,        transactions_hashes, serialized_abi };
    }
    catch (const std::exception& e) {
        LOG_ERROR << "deserialization error" << e.what();
        return std::nullopt;
    }
}


web::json::value serializeInfo(const Info& info)
{
    web::json::value result;
    result["top_block_hash"] = serializeHash(info.top_block_hash);
    result["top_block_number"] = web::json::value::number(info.top_block_number);
    result["api_version"] = web::json::value::number(info.api_version);
    result["peers_number"] = web::json::value::number(info.peers_number);
    return result;
}


std::optional<Info> deserializeInfo(const web::json::value& input)
{
    try {
        auto top_block_hash = deserializeHash(input.at("top_block_hash").as_string());
        auto top_block_number = input.at("top_block_number").as_number().to_uint64();
        auto api_version = input.at("api_version").as_number().to_uint32();
        auto peers_number = input.at("peers_number").as_number().to_uint64();

        return Info{ top_block_hash.value(), top_block_number, api_version, peers_number };
    }
    catch (const std::exception& e) {
        LOG_ERROR << "deserialization error" << e.what();
        return std::nullopt;
    }
}


web::json::value serializeTransaction(const lk::Transaction& input)
{
    web::json::value result;
    result["from"] = web::json::value::string(input.getFrom().toString());
    result["to"] = web::json::value::string(input.getTo().toString());
    result["amount"] = web::json::value::string(std::to_string(input.getAmount()));
    result["fee"] = web::json::value::string(std::to_string(input.getFee()));
    result["timestamp"] = web::json::value::number(input.getTimestamp().getSecondsSinceEpoch());

    web::json::value data_value;
    if (input.getTo() == lk::Address::null()) {
        base::SerializationIArchive io(input.getData());
        auto contract_data = lk::ContractData::deserialize(io);
        data_value["message"] = web::json::value::string(base::base64Encode(contract_data.getMessage()));
        data_value["abi"] = web::json::value::string(contract_data.getAbi().toString());
    }
    else {
        data_value["message"] = web::json::value::string(base::base64Encode(input.getData()));
        data_value["abi"] = web::json::value::string("");
    }

    result["data"] = data_value;

    result["sign"] = web::json::value::string(input.getSign().toBase64());
    return result;
}


std::optional<lk::Transaction> deserializeTransaction(const web::json::value& input)
{
    try {
        lk::TransactionBuilder txb;

        auto amount = deserializeBalance(input.at("amount").as_string());
        txb.setAmount(amount.value());

        auto fee = deserializeBalance(input.at("fee").as_string());
        txb.setFee(fee.value());

        auto from = deserializeAddress(input.at("from").as_string());
        txb.setFrom(from.value());

        auto to = deserializeAddress(input.at("to").as_string());
        txb.setTo(to.value());

        auto timestamp = base::Time(input.at("timestamp").as_number().to_uint32());
        txb.setTimestamp(timestamp);

        auto data = input.at("data");
        if (to == lk::Address::null()) {
            auto code = deserializeBytes(data.at("message").as_string());
            auto abi = base::parseJson(data.at("abi").as_string());
            lk::ContractData contract_data(code.value(), abi);
            txb.setData(base::toBytes(contract_data));
        }
        else {
            txb.setData(deserializeBytes(data.at("message").as_string()).value());
        }

        auto sign = lk::Sign::fromBase64(input.at("sign").as_string());
        txb.setSign(sign);

        auto tx = txb.build();
        return tx;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "deserialization error" << e.what();
        return std::nullopt;
    }
}


web::json::value serializeBlock(const lk::Block& block)
{
    web::json::value result;
    result["depth"] = web::json::value::number(block.getDepth());
    result["nonce"] = web::json::value::number(block.getNonce());
    result["coinbase"] = serializeAddress(block.getCoinbase());
    result["previous_block_hash"] = serializeHash(block.getPrevBlockHash());
    result["timestamp"] = web::json::value::number(block.getTimestamp().getSecondsSinceEpoch());

    std::vector<web::json::value> txs_values;
    for (auto& tx : block.getTransactions()) {
        txs_values.emplace_back(serializeTransaction(tx));
    }
    result["transactions"] = web::json::value::array(txs_values);
    return result;
}


std::optional<lk::Block> deserializeBlock(const web::json::value& input)
{
    try {
        auto depth = input.at("depth").as_number().to_uint64();
        auto nonce = input.at("nonce").as_number().to_uint64();
        auto timestamp = base::Time(input.at("timestamp").as_number().to_uint64());
        auto previous_block_hash = deserializeHash(input.at("previous_block_hash").as_string());
        auto coinbase = deserializeAddress(input.at("coinbase").as_string());

        lk::TransactionsSet txs;
        for (const auto& res_tx : input.at("transactions").as_array()) {
            txs.add(deserializeTransaction(res_tx).value());
        }
        lk::Block block{ depth, previous_block_hash.value(), timestamp, coinbase.value(), txs };
        block.setNonce(nonce);
        return block;
    }
    catch (const std::exception& e) {
        LOG_ERROR << "deserialization error" << e.what();
        return std::nullopt;
    }
}


web::json::value serializeTransactionStatus(const lk::TransactionStatus& status)
{
    web::json::value result;
    result["status_code"] = serializeTransactionStatusStatusCode(status.getStatus());
    result["action_type"] = serializeTransactionStatusActionType(status.getType());
    result["gas_left"] = web::json::value::string(std::to_string(status.getFeeLeft()));
    result["message"] = web::json::value::string(status.getMessage());
    return result;
}


std::optional<lk::TransactionStatus> deserializeTransactionStatus(const web::json::value& input)
{
    try {
        auto status = deserializeTransactionStatusStatusCode(input.at("status_code").as_number().to_uint32());
        auto type = deserializeTransactionStatusActionType(input.at("action_type").as_number().to_uint32());
        auto fee_left = deserializeBalance(input.at(U("gas_left")).as_string()).value();
        auto message = input.at("message").as_string();

        return lk::TransactionStatus{ status.value(), type.value(), fee_left, message };
    }
    catch (const std::exception& e) {
        LOG_ERROR << "deserialization error" << e.what();
        return std::nullopt;
    }
}


}