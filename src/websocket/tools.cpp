#include "tools.hpp"

#include "base/error.hpp"

#include <boost/spirit/include/qi.hpp>

namespace websocket
{

boost::asio::ip::tcp::endpoint createEndpoint(const std::string& listening_address)
{
    std::size_t i = listening_address.find(':');
    if (i == std::string_view::npos) {
        RAISE_ERROR(base::InvalidArgument, "port is not specified");
    }

    auto ip_address_part = listening_address.substr(0, i);
    auto port_part = listening_address.substr(i + 1, listening_address.length() - (i + 1));

    try {
        auto address = boost::asio::ip::make_address_v4(ip_address_part);
        unsigned short port = 0;
        if (!boost::spirit::qi::parse(std::cbegin(port_part), std::cend(port_part), boost::spirit::qi::int_, port)) {
            RAISE_ERROR(base::InvalidArgument, "can't find port number");
        }
        if (port == 0) {
            RAISE_ERROR(base::InvalidArgument, "can't find port number");
        }
        return boost::asio::ip::tcp::endpoint(address, port);
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InvalidArgument, std::string{ "invalid address: " } + std::string{ listening_address });
    }
}


std::string serializeCommandName(Command::Id command)
{
    LOG_TRACE << "Serializing CommandName";
    switch (Command::Name(static_cast<std::uint64_t>(command) & static_cast<std::uint64_t>(Command::NameMask))) {
        case Command::Name::ACCOUNT_INFO:
            return "account_info";
        case Command::Name::FIND_BLOCK:
            return "find_block";
        case Command::Name::FIND_TRANSACTION:
            return "find_transaction";
        case Command::Name::FIND_TRANSACTION_STATUS:
            return "find_transaction_status";
        case Command::Name::PUSH_TRANSACTION:
            return "push_transaction";
        case Command::Name::LAST_BLOCK_INFO:
            return "last_block_info";
        default:
            RAISE_ERROR(base::LogicError, "used unexpected command name");
    }
}


websocket::Command::Name deserializeCommandName(const std::string& command_name_str)
{
    LOG_TRACE << "Deserializing CommandName";
    if (command_name_str == "account_info") {
        return websocket::Command::Name::ACCOUNT_INFO;
    }
    if (command_name_str == "find_block") {
        return websocket::Command::Name::FIND_BLOCK;
    }
    if (command_name_str == "find_transaction") {
        return websocket::Command::Name::FIND_TRANSACTION;
    }
    if (command_name_str == "find_transaction_status") {
        return websocket::Command::Name::FIND_TRANSACTION_STATUS;
    }
    if (command_name_str == "push_transaction") {
        return websocket::Command::Name::PUSH_TRANSACTION;
    }
    if (command_name_str == "last_block_info") {
        return websocket::Command::Name::LAST_BLOCK_INFO;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("not any command name found by ") + command_name_str);
}


std::string serializeCommandType(websocket::Command::Id command_type)
{
    LOG_TRACE << "Serializing CommandType";
    switch (Command::Type(static_cast<std::uint64_t>(command_type) & static_cast<std::uint64_t>(Command::TypeMask))) {
        case Command::Type::CALL:
            return "call";
        case Command::Type::UNSUBSCRIBE:
            return "unsubscribe";
        case Command::Type::SUBSCRIBE:
            return "subscribe";
        default:
            RAISE_ERROR(base::LogicError, "used unexpected command type");
    }
}


websocket::Command::Type deserializeCommandType(const std::string& command_type_str)
{
    LOG_TRACE << "Deserializing CommandType";
    if (command_type_str == "call") {
        return websocket::Command::Type::CALL;
    }
    if (command_type_str == "unsubscribe") {
        return websocket::Command::Type::UNSUBSCRIBE;
    }
    if (command_type_str == "subscribe") {
        return websocket::Command::Type::SUBSCRIBE;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("not any command name found by ") + command_type_str);
}


std::string serializeAccountType(lk::AccountType account_type)
{
    LOG_TRACE << "Serializing AccountType";
    switch (account_type) {
        case lk::AccountType::CONTRACT:
            return "Contract";
        case lk::AccountType::CLIENT:
            return "Client";
        default:
            RAISE_ERROR(base::LogicError, "used unexpected account type");
    }
}


lk::AccountType deserializeAccountType(const std::string& account_type_str)
{
    LOG_TRACE << "Deserializing AccountType";
    if (account_type_str == "Client") {
        return lk::AccountType::CLIENT;
    }
    else if (account_type_str == "Contract") {
        return lk::AccountType::CONTRACT;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("Error in deserialization of AccountType: ") + account_type_str);
}


uint32_t serializeTransactionStatusStatusCode(lk::TransactionStatus::StatusCode status_code)
{
    LOG_TRACE << "Serializing TransactionStatusCode";
    return static_cast<uint32_t>(status_code);
}


lk::TransactionStatus::StatusCode deserializeTransactionStatusStatusCode(std::uint32_t status_code)
{
    LOG_TRACE << "Deserializing TransactionStatusCode";
    switch (status_code) {
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
            RAISE_ERROR(base::InvalidArgument,
                        std::string("Error in deserialization of StatusCode: ") + std::to_string(status_code));
    }
}


uint32_t serializeTransactionStatusActionType(lk::TransactionStatus::ActionType action_type)
{
    LOG_TRACE << "Serializing TransactionStatusActionType";
    return static_cast<uint32_t>(action_type);
}


lk::TransactionStatus::ActionType deserializeTransactionStatusActionType(std::uint32_t action_type)
{
    LOG_TRACE << "Deserializing TransactionStatusActionType";
    switch (action_type) {
        case 0:
            return lk::TransactionStatus::ActionType::None;
        case 1:
            return lk::TransactionStatus::ActionType::Transfer;
        case 2:
            return lk::TransactionStatus::ActionType::ContractCall;
        case 3:
            return lk::TransactionStatus::ActionType::ContractCreation;
        default:
            RAISE_ERROR(base::InvalidArgument,
                        std::string("Error in deserialization of ActionType: ") + std::to_string(action_type));
    }
}


std::string serializeBalance(const lk::Balance& balance)
{
    LOG_TRACE << "Serializing Balance";
    return balance.str();
}


lk::Balance deserializeBalance(const std::string& balance_str)
{
    LOG_TRACE << "Deserializing Balance";
    return lk::Balance{ balance_str };
}


std::string serializeFee(lk::Fee fee)
{
    LOG_TRACE << "Serializing Fee";
    return std::to_string(fee);
}


lk::Fee deserializeFee(const std::string& fee_str)
{
    LOG_TRACE << "Deserializing Fee";
    char* end = nullptr;
    auto fee = static_cast<uint64_t>(std::strtoll(fee_str.c_str(), &end, 10));
    if (end == nullptr) {
        RAISE_ERROR(base::InvalidArgument, std::string("Error in deserialization of Fee: ") + fee_str);
    }
    return fee;
}


std::string serializeHash(const base::Sha256& hash)
{
    LOG_TRACE << "Serializing Hash";
    return base::base64Encode(hash.getBytes());
}


base::Sha256 deserializeHash(const std::string& hash_str)
{
    LOG_TRACE << "Deserializing Hash";
    base::Bytes decoded_bytes;
    try {
        decoded_bytes = base::base64Decode(hash_str);
    }
    catch (const base::Error& e) {
        RAISE_ERROR(base::InvalidArgument, std::string("Error in deserialization of base64HashBytes: ") + hash_str);
    }

    return base::Sha256{ decoded_bytes };
}


std::string serializeAddress(const lk::Address& address)
{
    LOG_TRACE << "Serializing Address";
    return base::base58Encode(address.getBytes());
}


lk::Address deserializeAddress(const std::string& address_str)
{
    LOG_TRACE << "Deserializing Address";
    base::Bytes decoded_bytes;
    try {
        decoded_bytes = base::base58Decode(address_str);
    }
    catch (const base::Error& e) {
        RAISE_ERROR(base::InvalidArgument,
                    std::string("Error in deserialization of base58AddressBytes: ") + address_str);
    }

    return lk::Address{ decoded_bytes };
}


std::string serializeBytes(const base::Bytes& data)
{
    LOG_TRACE << "Serializing Bytes";
    return base::base64Encode(data);
}


base::Bytes deserializeBytes(const std::string& data_str)
{
    LOG_TRACE << "Deserializing Bytes";
    try {
        return base::base64Decode(data_str);
    }
    catch (const base::Error& e) {
        RAISE_ERROR(base::InvalidArgument, std::string("Error in deserialization of base64Bytes: ") + data_str);
    }
}


std::string serializeSign(const lk::Sign& sign)
{
    LOG_TRACE << "Serializing Sign";
    return base::base64Encode(sign.toBytes());
}


lk::Sign deserializeSign(const std::string& sign_str)
{
    LOG_TRACE << "Deserializing Sign";
    base::Bytes decoded_bytes;
    try {
        decoded_bytes = base::base64Decode(sign_str);
    }
    catch (const base::Error& e) {
        RAISE_ERROR(base::InvalidArgument, std::string("Error in deserialization of base64SignBytes: ") + sign_str);
    }

    return lk::Sign{ decoded_bytes };
}


void serializeAccountInfo(const lk::AccountInfo& account_info, rapidjson::Document& result)
{
    LOG_TRACE << "Serializing AccountInfo";
    auto& allocator = result.GetAllocator();

    auto address_value = serializeAddress(account_info.address);
    auto balance_value = serializeBalance(account_info.balance);
    auto type_value = serializeAccountType(account_info.type);
    rapidjson::Value txs_hashes_value(rapidjson::kArrayType);
    for (const auto& tx_hash : account_info.transactions_hashes) {
        auto hash_value = serializeHash(tx_hash);
        txs_hashes_value.PushBack(rapidjson::StringRef(hash_value.c_str()), allocator);
    }

    result.AddMember("address", rapidjson::StringRef(address_value.c_str()), allocator);
    result.AddMember("balance", rapidjson::StringRef(balance_value.c_str()), allocator);
    result.AddMember("nonce", rapidjson::Value(account_info.nonce), allocator);
    result.AddMember("type", rapidjson::StringRef(type_value.c_str()), allocator);
    result.AddMember("transaction_hashes", txs_hashes_value.Move(), allocator);
}


lk::AccountInfo deserializeAccountInfo(rapidjson::Value input)
{
    LOG_TRACE << "Deserializing AccountInfo";
    if (!input.HasMember("type")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain \"type\" member");
    }
    auto account_type_json_value = input.FindMember("type");
    if (!(account_type_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo \"type\" member is not a string type");
    }
    auto account_type = deserializeAccountType(account_type_json_value->value.GetString());

    if (!input.HasMember("balance")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain \"balance\" member");
    }
    auto balance_json_value = input.FindMember("balance");
    if (!(balance_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo \"balance\" member is not a string type");
    }
    auto balance = deserializeBalance(balance_json_value->value.GetString());

    if (!input.HasMember("nonce")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain \"nonce\" member");
    }
    auto nonce_json_value = input.FindMember("nonce");
    if (!(nonce_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo \"nonce\" member is not a uint type");
    }
    auto nonce = nonce_json_value->value.GetUint64();

    if (!input.HasMember("address")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain \"address\" member");
    }
    auto address_json_value = input.FindMember("address");
    if (!(address_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo \"address\" member is not a string type");
    }
    auto address = deserializeAddress(address_json_value->value.GetString());

    if (!input.HasMember("transaction_hashes")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain \"transaction_hashes\" member");
    }
    auto transaction_hashes_json_value = input.FindMember("transaction_hashes");
    if (!(transaction_hashes_json_value->value.IsArray())) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo \"transaction_hashes\" member is not an array type");
    }
    std::vector<base::Sha256> transactions_hashes;
    for (auto& tx_hash_value : transaction_hashes_json_value->value.GetArray()) {
        if (!tx_hash_value.IsString()) {
            RAISE_ERROR(base::InvalidArgument, "AccountInfo \"transaction_hash\" one member is not a string type");
        }
        transactions_hashes.emplace_back(deserializeHash(tx_hash_value.GetString()));
    }

    return lk::AccountInfo{
        account_type, std::move(address), std::move(balance), nonce, std::move(transactions_hashes)
    };
}


void serializeInfo(const NodeInfo& info, rapidjson::Document& result)
{
    LOG_TRACE << "Serializing NodeInfo";
    auto& allocator = result.GetAllocator();

    auto hash_value = serializeHash(info.top_block_hash);
    auto val = rapidjson::StringRef(hash_value.c_str());
    auto test_1 = hash_value.size();
    auto test_2 = val.length;

    result.AddMember("top_block_number", rapidjson::Value(info.top_block_number), allocator);
    result.AddMember("top_block_hash", val, allocator);


}


NodeInfo deserializeInfo(rapidjson::Value input)
{
    LOG_TRACE << "Deserializing NodeInfo";
    if (!input.HasMember("top_block_hash")) {
        RAISE_ERROR(base::InvalidArgument, "NodeInfo json is not contain \"top_block_hash\" member");
    }
    auto top_block_hash_json_value = input.FindMember("top_block_hash");
    if (!(top_block_hash_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "NodeInfo \"top_block_hash\" member is not a string type");
    }
    auto top_block_hash = deserializeHash(top_block_hash_json_value->value.GetString());

    if (!input.HasMember("top_block_number")) {
        RAISE_ERROR(base::InvalidArgument, "NodeInfo json is not contain \"top_block_number\" member");
    }
    auto top_block_number_json_value = input.FindMember("top_block_number");
    if (!(top_block_number_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "NodeInfo \"top_block_number\" member is not a uint type");
    }
    auto top_block_number = top_block_number_json_value->value.GetUint64();

    return NodeInfo{ top_block_hash, top_block_number };
}


void serializeTransaction(const lk::Transaction& input, rapidjson::Document& result)
{
    LOG_TRACE << "Serializing Transaction";
    auto& allocator = result.GetAllocator();

    auto from_address_value = serializeAddress(input.getFrom());
    auto to_address_value = serializeAddress(input.getTo());
    auto amount_value = serializeBalance(input.getAmount());
    auto fee_value = serializeFee(input.getFee());
    auto timestamp_value = input.getTimestamp().getSeconds();
    auto data_value = serializeBytes(input.getData());
    auto sign_value = serializeSign(input.getSign());

    result.AddMember("from", rapidjson::StringRef(from_address_value.c_str()), allocator);
    result.AddMember("to", rapidjson::StringRef(to_address_value.c_str()), allocator);
    result.AddMember("amount", rapidjson::StringRef(amount_value.c_str()), allocator);
    result.AddMember("fee", rapidjson::StringRef(fee_value.c_str()), allocator);
    result.AddMember("timestamp", rapidjson::Value(timestamp_value), allocator);
    result.AddMember("sign", rapidjson::StringRef(data_value.c_str()), allocator);
    result.AddMember("data", rapidjson::StringRef(sign_value.c_str()), allocator);
}


lk::Transaction deserializeTransaction(rapidjson::Value input)
{
    LOG_TRACE << "Deserializing Transaction";
    if (!input.HasMember("amount")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"amount\" member");
    }
    auto amount_json_value = input.FindMember("amount");
    if (!(amount_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"amount\" member is not a string type");
    }
    auto amount = deserializeBalance(amount_json_value->value.GetString());

    if (!input.HasMember("fee")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"fee\" member");
    }
    auto fee_json_value = input.FindMember("fee");
    if (!(fee_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"fee\" member is not a string type");
    }
    auto fee = deserializeFee(fee_json_value->value.GetString());

    if (!input.HasMember("from")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"from\" member");
    }
    auto from_json_value = input.FindMember("from");
    if (!(from_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"from\" member is not a string type");
    }
    auto from = deserializeAddress(from_json_value->value.GetString());

    if (!input.HasMember("to")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"to\" member");
    }
    auto to_json_value = input.FindMember("to");
    if (!(to_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"to\" member is not a string type");
    }
    auto to = deserializeAddress(to_json_value->value.GetString());

    if (!input.HasMember("timestamp")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"timestamp\" member");
    }
    auto timestamp_json_value = input.FindMember("timestamp");
    if (!(timestamp_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"timestamp\" member is not a uint type");
    }
    auto timestamp = base::Time(timestamp_json_value->value.GetUint64());

    if (!input.HasMember("data")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"data\" member");
    }
    auto data_json_value = input.FindMember("data");
    if (!(data_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"data\" member is not a string type");
    }
    auto data = deserializeBytes(data_json_value->value.GetString());

    if (!input.HasMember("sign")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"sign\" member");
    }
    auto sign_json_value = input.FindMember("sign");
    if (!(sign_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"sign\" member is not a string type");
    }
    auto sign = deserializeSign(sign_json_value->value.GetString());

    return lk::Transaction{ std::move(from), std::move(to),   std::move(amount), fee,
                            timestamp,       std::move(data), std::move(sign) };
}


void serializeBlock(const lk::ImmutableBlock& block, rapidjson::Document& result)
{
    LOG_TRACE << "Serializing ImmutableBlock";
    auto& allocator = result.GetAllocator();

    auto coinbase_value = serializeAddress(block.getCoinbase());
    auto previous_block_hash_value = serializeHash(block.getPrevBlockHash());
    rapidjson::Value txs_values(rapidjson::kArrayType);
    for (auto& tx : block.getTransactions()) {
        rapidjson::Document tx_value(rapidjson::kObjectType);
        serializeTransaction(tx, tx_value);
        txs_values.PushBack(tx_value, allocator);
    }

    result.AddMember("depth", rapidjson::Value(block.getDepth()), allocator);
    result.AddMember("nonce", rapidjson::Value(block.getNonce()), allocator);
    result.AddMember("coinbase", rapidjson::StringRef(coinbase_value.c_str()), allocator);
    result.AddMember("previous_block_hash", rapidjson::StringRef(previous_block_hash_value.c_str()), allocator);
    result.AddMember("timestamp", rapidjson::Value(block.getTimestamp().getSeconds()), allocator);
    result.AddMember("transactions", txs_values.Move(), allocator);
}


lk::ImmutableBlock deserializeBlock(rapidjson::Value input)
{
    LOG_TRACE << "Deserializing ImmutableBlock";

    if (!input.HasMember("depth")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"depth\" member");
    }
    auto depth_json_value = input.FindMember("depth");
    if (!(depth_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"depth\" member is not a uint type");
    }
    auto depth = depth_json_value->value.GetUint64();

    if (!input.HasMember("nonce")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"nonce\" member");
    }
    auto nonce_json_value = input.FindMember("nonce");
    if (!(nonce_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"nonce\" member is not a uint type");
    }
    auto nonce = nonce_json_value->value.GetUint64();

    if (!input.HasMember("timestamp")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"timestamp\" member");
    }
    auto timestamp_json_value = input.FindMember("timestamp");
    if (!(timestamp_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"timestamp\" member is not a uint type");
    }
    auto timestamp = base::Time(timestamp_json_value->value.GetUint64());

    if (!input.HasMember("previous_block_hash")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"previous_block_hash\" member");
    }
    auto previous_block_hash_json_value = input.FindMember("previous_block_hash");
    if (!(previous_block_hash_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"previous_block_hash\" member is not a string type");
    }
    auto previous_block_hash = deserializeHash(previous_block_hash_json_value->value.GetString());

    if (!input.HasMember("coinbase")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"coinbase\" member");
    }
    auto coinbase_json_value = input.FindMember("coinbase");
    if (!(coinbase_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"coinbase\" member is not a string type");
    }
    auto coinbase = deserializeAddress(coinbase_json_value->value.GetString());

    if (!input.HasMember("transactions")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain \"transactions\" member");
    }
    auto transactions_json_value = input.FindMember("transactions");
    if (!(transactions_json_value->value.IsArray())) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"transactions\" member is not an array type");
    }
    lk::TransactionsSet transactions;
    for (auto& txs_value : transactions_json_value->value.GetArray()) {
        transactions.add(std::move(deserializeTransaction(txs_value.GetObject())));
    }

    lk::BlockBuilder b;
    b.setDepth(depth);
    b.setNonce(nonce);
    b.setPrevBlockHash(std::move(previous_block_hash));
    b.setTimestamp(timestamp);
    b.setCoinbase(std::move(coinbase));
    b.setTransactionsSet(std::move(transactions));
    return std::move(b).buildImmutable();
}


void serializeTransactionStatus(const lk::TransactionStatus& status, rapidjson::Document& result)
{
    LOG_TRACE << "Serializing TransactionStatus";
    auto& allocator = result.GetAllocator();

    auto status_code_value = serializeTransactionStatusStatusCode(status.getStatus());
    auto action_type_value = serializeTransactionStatusActionType(status.getType());
    auto fee_left_value = serializeFee(status.getFeeLeft());

    result.AddMember("status_code", rapidjson::Value(status_code_value), allocator);
    result.AddMember("action_type", rapidjson::Value(action_type_value), allocator);
    result.AddMember("fee_left", rapidjson::StringRef(fee_left_value.c_str()), allocator);
    result.AddMember("message", rapidjson::StringRef(status.getMessage().c_str()), allocator);
}


lk::TransactionStatus deserializeTransactionStatus(rapidjson::Value input)
{
    LOG_TRACE << "Deserializing TransactionStatus";

    if (!input.HasMember("status_code")) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus json is not contain \"status_code\" member");
    }
    auto status_code_json_value = input.FindMember("status_code");
    if (!(status_code_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus \"status_code\" member is not a uint type");
    }
    auto status_code = deserializeTransactionStatusStatusCode(status_code_json_value->value.GetUint64());

    if (!input.HasMember("action_type")) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus json is not contain \"action_type\" member");
    }
    auto action_type_json_value = input.FindMember("action_type");
    if (!(action_type_json_value->value.IsUint64())) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus \"action_type\" member is not a uint type");
    }
    auto action_type = deserializeTransactionStatusActionType(action_type_json_value->value.GetUint64());

    if (!input.HasMember("fee")) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus json is not contain \"fee\" member");
    }
    auto fee_json_value = input.FindMember("fee");
    if (!(fee_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus \"fee\" member is not a string type");
    }
    auto fee = deserializeFee(fee_json_value->value.GetString());


    if (!input.HasMember("message")) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus json is not contain \"message\" member");
    }
    auto message_json_value = input.FindMember("message");
    if (!(message_json_value->value.IsString())) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus \"message\" member is not a string type");
    }
    std::string message{ fee_json_value->value.GetString() };

    return lk::TransactionStatus{ status_code, action_type, fee, message };
}

}