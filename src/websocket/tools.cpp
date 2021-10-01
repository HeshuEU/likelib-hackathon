#include "tools.hpp"

#include "base/error.hpp"

#include <boost/spirit/include/qi.hpp>

namespace
{
std::string toLkl(const lk::Balance& tokens)
{
    lk::Balance token_value{ base::config::BC_TOKEN_VALUE };
    std::string tokens_str;
    if (tokens >= token_value) {
        std::size_t count_left_numbers = tokens.str().size() - token_value.str().size() + 1;
        tokens_str = tokens.str().substr(0, count_left_numbers) + '.' + tokens.str().substr(count_left_numbers);
    }
    else {
        std::size_t count_zero = token_value.str().size() - tokens.str().size() - 1;
        tokens_str = "0.";
        for (std::size_t i = 0; i < count_zero; i++) {
            tokens_str += '0';
        }
        tokens_str += tokens.str();
    }
    tokens_str += base::config::BC_TOKEN_NAME;
    return tokens_str;
}


lk::Balance fromLkl(const std::string& lkl_tokens)
{ // TODO
    if (lkl_tokens.find('.') != std::string::npos &&
        lkl_tokens.substr(lkl_tokens.size() - strlen(base::config::BC_TOKEN_NAME)) == base::config::BC_TOKEN_NAME) {
        std::string tokens{ lkl_tokens };
        tokens.erase(tokens.size() - strlen(base::config::BC_TOKEN_NAME));
        std::size_t lkl_size = std::to_string(base::config::BC_TOKEN_VALUE).size() - 1;
        std::size_t numbers_size_after_dot = tokens.size() - tokens.find('.') - 1;
        for (numbers_size_after_dot; numbers_size_after_dot < lkl_size; numbers_size_after_dot++) {
            tokens += '0';
        }
        tokens.erase(tokens.find('.'), 1);

        while (tokens[0] == '0') {
            tokens.erase(0, 1);
        }
        return lk::Balance{ tokens };
    }
    return lk::Balance{ lkl_tokens };
}

}


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


base::json::Value serializeLogin(const std::string& login)
{
    LOG_TRACE << "Serializing Login";
    auto result = base::json::Value::object();
    result["login"] = base::json::Value::string(login);
    return result;
}


std::string deserializeLogin(const std::string& message)
{
    LOG_TRACE << "Deserializing Login";
    return message;
}


base::json::Value serializeMessage(const std::string& message)
{
    LOG_TRACE << "Serializing Message";
    auto result = base::json::Value::object();
    result["message"] = base::json::Value::string(message);
    return result;
}


std::string deserializeMessage(const std::string& message)
{
    LOG_TRACE << "Deserializing Message";
    return message;
}


base::json::Value serializeSessionId(const websocket::SessionId& session_id)
{
    LOG_TRACE << "Serializing Session Id";
    auto result = base::json::Value::object();
    result["sessionId"] = base::json::Value::string(std::to_string(session_id));
    return result;
}


websocket::SessionId deserializeSessionId(const std::string& message)
{
    LOG_TRACE << "Deserializing Session Id";
    char* end = nullptr;
    auto session_id = static_cast<uint64_t>(std::strtoll(message.c_str(), &end, 10));
    if (end == nullptr) {
        RAISE_ERROR(base::InvalidArgument, std::string("Error in deserialization of Fee: ") + message);
    }
    return session_id;
}

// FEE_INFO
base::json::Value serializeCommandName(Command::Id command)
{
    LOG_TRACE << "Serializing CommandName";
    switch (Command::Name(static_cast<std::uint64_t>(command) & static_cast<std::uint64_t>(Command::NameMask))) {
        case Command::Name::FEE_INFO:
            return base::json::Value::string("fee_info");
        case Command::Name::ACCOUNT_INFO:
            return base::json::Value::string("account_info");
        case Command::Name::FIND_BLOCK:
            return base::json::Value::string("find_block");
        case Command::Name::FIND_TRANSACTION:
            return base::json::Value::string("find_transaction");
        case Command::Name::FIND_TRANSACTION_STATUS:
            return base::json::Value::string("find_transaction_status");
        case Command::Name::PUSH_TRANSACTION:
            return base::json::Value::string("push_transaction");
        case Command::Name::LAST_BLOCK_INFO:
            return base::json::Value::string("last_block_info");
        case Command::Name::LOGIN:
            return base::json::Value::string("login");
        default:
            RAISE_ERROR(base::LogicError, "used unexpected command name");
    }
}


websocket::Command::Name deserializeCommandName(const std::string& command_name_str)
{
    LOG_TRACE << "Deserializing CommandName";
    if (command_name_str == "fee_info") {
        return websocket::Command::Name::FEE_INFO;
    }
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
    if (command_name_str == "login") {
        return websocket::Command::Name::LOGIN;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("not any command name found by ") + command_name_str);
}


base::json::Value serializeCommandType(websocket::Command::Id command_type)
{
    LOG_TRACE << "Serializing CommandType";
    switch (Command::Type(static_cast<std::uint64_t>(command_type) & static_cast<std::uint64_t>(Command::TypeMask))) {
        case Command::Type::CALL:
            return base::json::Value::string("call");
        case Command::Type::UNSUBSCRIBE:
            return base::json::Value::string("unsubscribe");
        case Command::Type::SUBSCRIBE:
            return base::json::Value::string("subscribe");
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


base::json::Value serializeAccountType(lk::AccountType account_type)
{
    LOG_TRACE << "Serializing AccountType";
    switch (account_type) {
        case lk::AccountType::CONTRACT:
            return base::json::Value::string("Contract");
        case lk::AccountType::CLIENT:
            return base::json::Value::string("Client");
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


base::json::Value serializeTransactionStatusStatusCode(lk::TransactionStatus::StatusCode status_code)
{
    LOG_TRACE << "Serializing TransactionStatusCode";
    return base::json::Value::number(static_cast<uint32_t>(status_code));
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


base::json::Value serializeTransactionStatusActionType(lk::TransactionStatus::ActionType action_type)
{
    LOG_TRACE << "Serializing TransactionStatusActionType";
    return base::json::Value::number(static_cast<uint32_t>(action_type));
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


base::json::Value serializeDepth(const lk::BlockDepth& depth)
{
    LOG_TRACE << "Serializing Depth";
    return base::json::Value::number(depth);
}


lk::BlockDepth deserializeDepth(const std::string& depth_str)
{
    LOG_TRACE << "Deserializing Depth";
    char* end = nullptr;
    auto depth = static_cast<uint64_t>(std::strtoll(depth_str.c_str(), &end, 10));
    if (end == nullptr) {
        RAISE_ERROR(base::InvalidArgument, std::string("Error in deserialization of Fee: ") + depth_str);
    }
    return depth;
}


base::json::Value serializeBalance(const lk::Balance& balance)
{
    LOG_TRACE << "Serializing Balance";
    auto lkl_balance{ toLkl(balance) };
    return base::json::Value::string(lkl_balance);
}


lk::Balance deserializeBalance(const std::string& balance_str)
{
    LOG_TRACE << "Deserializing Balance";
    return lk::Balance{ fromLkl(balance_str) };
}


base::json::Value serializeFee(lk::Fee fee)
{
    LOG_TRACE << "Serializing Fee";
    return base::json::Value::string(std::to_string(fee));
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


base::json::Value serializeHash(const base::Sha256& hash)
{
    LOG_TRACE << "Serializing Hash";
    return base::json::Value::string(base::toHex(hash.getBytes()));
}


base::Sha256 deserializeHash(const std::string& hash_str)
{
    LOG_TRACE << "Deserializing Hash";
    base::Bytes decoded_bytes;
    try {
        decoded_bytes = base::fromHex<base::Bytes>(hash_str);
    }
    catch (const base::Error& e) {
        RAISE_ERROR(base::InvalidArgument, std::string("Error in deserialization of base64HashBytes: ") + hash_str);
    }

    return base::Sha256{ decoded_bytes };
}


base::json::Value serializeAddress(const lk::Address& address)
{
    LOG_TRACE << "Serializing Address";
    return base::json::Value::string(base::base58Encode(address.getBytes()));
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


base::json::Value serializeBytes(const base::Bytes& data)
{
    LOG_TRACE << "Serializing Bytes";
    return base::json::Value::string(base::base64Encode(data));
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


base::json::Value serializeSign(const lk::Sign& sign)
{
    LOG_TRACE << "Serializing Sign";
    return base::json::Value::string(base::base64Encode(sign.toBytes()));
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


base::json::Value serializeAccountInfo(const lk::AccountInfo& account_info)
{
    LOG_TRACE << "Serializing AccountInfo";
    auto result = base::json::Value::object();
    result["address"] = serializeAddress(account_info.address);
    result["balance"] = serializeBalance(account_info.balance);
    result["nonce"] = base::json::Value::number(account_info.nonce);
    result["type"] = serializeAccountType(account_info.type);
    std::vector<base::json::Value> txs_hashes_value;

    for (const auto& tx_hash : account_info.transactions_hashes) {
        txs_hashes_value.emplace_back(std::move(serializeHash(tx_hash)));
    }
    result["transaction_hashes"] = base::json::Value::array(txs_hashes_value);
    return result;
}


lk::AccountInfo deserializeAccountInfo(base::json::Value input)
{
    LOG_TRACE << "Deserializing AccountInfo";
    if (!input.has_string_field("type")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain a string \"type\" member");
    }
    auto account_type = deserializeAccountType(input["type"].as_string());

    if (!input.has_string_field("balance")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain a \"balance\" member");
    }
    auto balance = deserializeBalance(input["balance"].as_string());

    if (!input.has_number_field("nonce")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain an uint \"nonce\" member");
    }
    auto nonce_json_value = input["nonce"].as_number();
    if (!nonce_json_value.is_uint64()) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo \"nonce\" member is not a uint type");
    }
    auto nonce = nonce_json_value.to_uint64();

    if (!input.has_string_field("address")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain a string \"address\" member");
    }
    auto address = deserializeAddress(input["address"].as_string());

    if (!input.has_array_field("transaction_hashes")) {
        RAISE_ERROR(base::InvalidArgument, "AccountInfo json is not contain an array \"transaction_hashes\" member");
    }
    std::vector<base::Sha256> transactions_hashes;
    for (auto& tx_hash_value : input["transaction_hashes"].as_array()) {
        if (!tx_hash_value.is_string()) {
            RAISE_ERROR(base::InvalidArgument, "AccountInfo \"transaction_hash\" one member is not a string type");
        }
        transactions_hashes.emplace_back(deserializeHash(tx_hash_value.as_string()));
    }

    return lk::AccountInfo{
        account_type, std::move(address), std::move(balance), nonce, std::move(transactions_hashes)
    };
}


base::json::Value serializeInfo(const NodeInfo& info)
{
    LOG_TRACE << "Serializing NodeInfo";
    auto result = base::json::Value::object();
    result["top_block_number"] = base::json::Value::number(info.top_block_number);
    result["top_block_hash"] = serializeHash(info.top_block_hash);
    return result;
}


NodeInfo deserializeInfo(base::json::Value input)
{
    LOG_TRACE << "Deserializing NodeInfo";
    if (!input.has_string_field("top_block_hash")) {
        RAISE_ERROR(base::InvalidArgument, "NodeInfo json is not contain a string \"top_block_hash\" member");
    }
    auto top_block_hash = deserializeHash(input["top_block_hash"].as_string());

    if (!input.has_number_field("top_block_number")) {
        RAISE_ERROR(base::InvalidArgument, "NodeInfo json is not contain a uint \"top_block_number\" member");
    }
    auto top_block_number_json_value = input["top_block_number"].as_number();
    if (!top_block_number_json_value.is_uint64()) {
        RAISE_ERROR(base::InvalidArgument, "NodeInfo \"top_block_number\" member is not a uint type");
    }
    auto top_block_number = top_block_number_json_value.to_uint64();

    return NodeInfo{ top_block_hash, top_block_number };
}


base::json::Value serializeTransaction(const lk::Transaction& input)
{
    LOG_TRACE << "Serializing Transaction";
    auto result = base::json::Value::object();

    result["from"] = serializeAddress(input.getFrom());
    result["to"] = serializeAddress(input.getTo());
    result["amount"] = serializeBalance(input.getAmount());
    result["fee"] = serializeFee(input.getFee());
    result["timestamp"] = base::json::Value::number(input.getTimestamp().getSeconds());
    result["sign"] = serializeSign(input.getSign());
    result["data"] = serializeBytes(input.getData());
    return result;
}


lk::Transaction deserializeTransaction(base::json::Value input)
{
    LOG_TRACE << "Deserializing Transaction";
    if (!input.has_string_field("amount")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a string \"amount\" member");
    }
    auto amount = deserializeBalance(input["amount"].as_string());

    if (!input.has_string_field("fee")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a string \"fee\" member");
    }
    auto fee = deserializeFee(input["fee"].as_string());

    if (!input.has_string_field("from")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a string \"from\" member");
    }
    auto from = deserializeAddress(input["from"].as_string());

    if (!input.has_string_field("to")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a string \"to\" member");
    }
    auto to = deserializeAddress(input["to"].as_string());

    if (!input.has_number_field("timestamp")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a uint \"timestamp\" member");
    }
    auto timestamp_json_value = input["timestamp"].as_number();
    if (!timestamp_json_value.is_uint32()) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"timestamp\" member is not a uint type");
    }
    auto timestamp = base::Time(timestamp_json_value.to_uint32());

    if (!input.has_string_field("data")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a string \"data\" member");
    }
    auto data = deserializeBytes(input["data"].as_string());

    if (!input.has_string_field("sign")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a string\"sign\" member");
    }
    auto sign = deserializeSign(input["sign"].as_string());

    return lk::Transaction{ std::move(from), std::move(to),   std::move(amount), fee,
                            timestamp,       std::move(data), std::move(sign) };
}


base::json::Value serializeBlock(const lk::ImmutableBlock& block)
{
    LOG_TRACE << "Serializing ImmutableBlock";
    auto result = base::json::Value::object();

    std::vector<base::json::Value> txs_values;
    for (auto& tx : block.getTransactions()) {
        txs_values.emplace_back(std::move(serializeTransaction(tx)));
    }

    result["depth"] = base::json::Value::number(block.getDepth());
    result["nonce"] = base::json::Value::number(block.getNonce());
    result["coinbase"] = serializeAddress(block.getCoinbase());
    result["previous_block_hash"] = serializeHash(block.getPrevBlockHash());
    result["timestamp"] = base::json::Value::number(block.getTimestamp().getSeconds());
    result["transactions"] = base::json::Value::array(txs_values);
    return result;
}


base::json::Value serializeMidFee(const lk::ImmutableBlock& block)
{
    LOG_TRACE << "Serializing MiddleFee";

    base::Uint256 all_fee{ 0 };
    for (auto& tx : block.getTransactions()) {
        all_fee += tx.getFee();
    }

    return base::json::Value::string((all_fee / block.getTransactions().size()).str());
}


lk::ImmutableBlock deserializeBlock(base::json::Value input)
{
    LOG_TRACE << "Deserializing ImmutableBlock";

    if (!input.has_number_field("depth")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a uint \"depth\" member");
    }
    auto depth_json_value = input["depth"].as_number();
    if (!depth_json_value.is_uint64()) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"depth\" member is not a uint type");
    }
    auto depth = depth_json_value.to_uint64();

    if (!input.has_number_field("nonce")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a uint \"nonce\" member");
    }
    auto nonce_json_value = input["nonce"].as_number();
    if (!nonce_json_value.is_uint64()) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"nonce\" member is not a uint type");
    }
    auto nonce = nonce_json_value.to_uint64();

    if (!input.has_number_field("timestamp")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a uint \"timestamp\" member");
    }
    auto timestamp_json_value = input["timestamp"].as_number();
    if (!timestamp_json_value.is_uint64()) {
        RAISE_ERROR(base::InvalidArgument, "Transaction \"timestamp\" member is not a uint type");
    }
    auto timestamp = base::Time(timestamp_json_value.to_uint64());

    if (!input.has_string_field("previous_block_hash")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a string \"previous_block_hash\" member");
    }
    auto previous_block_hash = deserializeHash(input["previous_block_hash"].as_string());

    if (!input.has_string_field("coinbase")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain a string \"coinbase\" member");
    }
    auto coinbase = deserializeAddress(input["coinbase"].as_string());

    if (!input.has_array_field("transactions")) {
        RAISE_ERROR(base::InvalidArgument, "Transaction json is not contain an array \"transactions\" member");
    }
    lk::TransactionsSet transactions;
    for (auto& txs_value : input["transactions"].as_array()) {
        transactions.add(std::move(deserializeTransaction(txs_value)));
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


base::json::Value serializeTransactionStatus(const lk::TransactionStatus& status)
{
    LOG_TRACE << "Serializing TransactionStatus";
    auto result = base::json::Value::object();
    result["status_code"] = serializeTransactionStatusStatusCode(status.getStatus());
    result["action_type"] = serializeTransactionStatusActionType(status.getType());
    result["fee_left"] = serializeFee(status.getFeeLeft());
    result["message"] = base::json::Value::string(status.getMessage());
    return result;
}


lk::TransactionStatus deserializeTransactionStatus(base::json::Value input)
{
    LOG_TRACE << "Deserializing TransactionStatus";

    if (!input.has_number_field("status_code")) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus json is not contain a uint \"status_code\" member");
    }
    auto status_code_json_value = input["status_code"].as_number();
    if (!status_code_json_value.is_uint64()) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus \"status_code\" member is not a uint type");
    }
    auto status_code = deserializeTransactionStatusStatusCode(status_code_json_value.to_uint64());

    if (!input.has_number_field("action_type")) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus json is not contain a uint \"action_type\" member");
    }
    auto action_type_json_value = input["action_type"].as_number();
    if (!action_type_json_value.is_uint64()) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus \"action_type\" member is not a uint type");
    }
    auto action_type = deserializeTransactionStatusActionType(action_type_json_value.to_uint64());

    if (!input.has_string_field("fee")) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus json is not contain a string \"fee\" member");
    }
    auto fee = deserializeFee(input["fee"].as_string());

    if (!input.has_string_field("message")) {
        RAISE_ERROR(base::InvalidArgument, "TransactionStatus json is not contain a string \"message\" member");
    }
    std::string message{ input["message"].as_string() };

    return lk::TransactionStatus{ status_code, action_type, fee, message };
}

}