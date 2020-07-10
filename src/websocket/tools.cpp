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


std::string serializeCommandName(Command::Id name)
{
    switch (Command::Name(static_cast<std::uint64_t>(name) & static_cast<std::uint64_t>(Command::NameMask))) {
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
    ASSERT(false);
}


websocket::Command::Name deserializeCommandName(const std::string& message)
{
    if (message == "account_info") {
        return websocket::Command::Name::ACCOUNT_INFO;
    }
    if (message == "find_block") {
        return websocket::Command::Name::FIND_BLOCK;
    }
    if (message == "find_transaction") {
        return websocket::Command::Name::FIND_TRANSACTION;
    }
    if (message == "find_transaction_status") {
        return websocket::Command::Name::FIND_TRANSACTION_STATUS;
    }
    if (message == "push_transaction") {
        return websocket::Command::Name::PUSH_TRANSACTION;
    }
    if (message == "last_block_info") {
        return websocket::Command::Name::LAST_BLOCK_INFO;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("not any type found by name") + message);
}


std::string serializeCommandType(websocket::Command::Id command_type)
{
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
    ASSERT(false);
}


websocket::Command::Type deserializeCommandType(const std::string& message)
{
    if (message == "call") {
        return websocket::Command::Type::CALL;
    }
    if (message == "unsubscribe") {
        return websocket::Command::Type::UNSUBSCRIBE;
    }
    if (message == "subscribe") {
        return websocket::Command::Type::SUBSCRIBE;
    }
    RAISE_ERROR(base::InvalidArgument, std::string("not any type found by name") + message);
}


const std::string& serializeAccountType(lk::AccountType type)
{
    switch (type) {
        case lk::AccountType::CONTRACT:
            static const std::string contract_type("Contract");
            return contract_type;
        case lk::AccountType::CLIENT:
            static const std::string client_type("Client");
            return client_type;
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


uint32_t serializeTransactionStatusStatusCode(lk::TransactionStatus::StatusCode status_code)
{
    return static_cast<uint32_t>(status_code);
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


uint32_t serializeTransactionStatusActionType(lk::TransactionStatus::ActionType action_type)
{
    return static_cast<uint32_t>(action_type);
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


std::string serializeBalance(const lk::Balance& balance)
{
    return balance.str();
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


std::string serializeFee(std::uint64_t balance)
{
    return std::to_string(balance);
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


std::string serializeHash(const base::Sha256& hash)
{
    return base::base64Encode(hash.getBytes());
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


std::string serializeAddress(const lk::Address& address)
{
    return base::base58Encode(address.getBytes());
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


std::string serializeBytes(const base::Bytes& data)
{
    return base::base64Encode(data);
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


std::string serializeSign(const lk::Sign& sign)
{
    return base::base64Encode(sign.toBytes());
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


base::PropertyTree serializeAccountInfo(const lk::AccountInfo& account_info)
{
    base::PropertyTree result;
    result.add("address", serializeAddress(account_info.address));
    result.add("balance", serializeBalance(account_info.balance));
    result.add("nonce", account_info.nonce);
    result.add("type", serializeAccountType(account_info.type));
    base::PropertyTree txs_hashes;
    for (const auto& tx_hash : account_info.transactions_hashes) {
        txs_hashes.add("", serializeHash(tx_hash));
    }
    result.add("transaction_hashes", std::move(txs_hashes));
    return result;
}


std::optional<lk::AccountInfo> deserializeAccountInfo(const base::PropertyTree& input)
{
    try {
        std::optional<lk::AccountType> type;
        if (input.hasKey("type")) {
            type = deserializeAccountType(input.get<std::string>("type"));
        }
        else {
            LOG_ERROR << "type field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Balance> balance;
        if (input.hasKey("balance")) {
            balance = deserializeBalance(input.get<std::string>("balance"));
        }
        else {
            LOG_ERROR << "balance field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> nonce;
        if (input.hasKey("nonce")) {
            nonce = input.get<uint64_t>("nonce");
        }
        else {
            LOG_ERROR << "nonce field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Address> address;
        if (input.hasKey("address")) {
            address = deserializeAddress(input.get<std::string>("address"));
        }
        else {
            LOG_ERROR << "address field is not exists";
            return std::nullopt;
        }

        std::vector<base::Sha256> transactions_hashes;
        if (input.hasKey("transaction_hashes")) {
            for (const auto& res_tx_hash : input.getSubTree("transaction_hashes")) {
                auto hash_opt = deserializeHash(res_tx_hash.second.get_value<std::string>());
                if (hash_opt) {
                    transactions_hashes.emplace_back(*hash_opt);
                }
                else {
                    LOG_ERROR << "error at hash deserialization";
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


base::PropertyTree serializeInfo(const NodeInfo& info)
{
    base::PropertyTree result;
    result.add("top_block_hash", serializeHash(info.top_block_hash));
    result.add("top_block_number", info.top_block_number);
    return result;
}


std::optional<NodeInfo> deserializeInfo(const base::PropertyTree& input)
{
    try {
        std::optional<base::Sha256> top_block_hash;
        if (input.hasKey("top_block_hash")) {
            top_block_hash = deserializeHash(input.get<std::string>("top_block_hash"));
        }
        else {
            LOG_ERROR << "top_block_hash field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> top_block_number;
        if (input.hasKey("top_block_number")) {
            top_block_number = input.get<std::uint64_t>("top_block_number");
        }
        else {
            LOG_ERROR << "top_block_number field is not exists";
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
        return NodeInfo{ top_block_hash.value(), top_block_number.value() };
    }
    catch (const std::exception& e) {
        LOG_ERROR << "Failed to deserialize Info";
        return std::nullopt;
    }
}


base::PropertyTree serializeTransaction(const lk::Transaction& input)
{
    base::PropertyTree result;
    result.add("from", serializeAddress(input.getFrom()));
    result.add("to", serializeAddress(input.getTo()));
    result.add("amount", serializeBalance(input.getAmount()));
    result.add("fee", serializeFee(input.getFee()));
    result.add("timestamp", input.getTimestamp().getSeconds());
    result.add("data", serializeBytes(input.getData()));
    result.add("sign", serializeSign(input.getSign()));
    return result;
}


std::optional<lk::Transaction> deserializeTransaction(const base::PropertyTree& input)
{
    try {
        std::optional<lk::Balance> amount;
        if (input.hasKey("amount")) {
            amount = deserializeBalance(input.get<std::string>("amount"));
        }
        else {
            LOG_ERROR << "amount field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> fee;
        if (input.hasKey("fee")) {
            fee = deserializeFee(input.get<std::string>("fee"));
        }
        else {
            LOG_ERROR << "fee field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Address> from;
        if (input.hasKey("from")) {
            from = deserializeAddress(input.get<std::string>("from"));
        }
        else {
            LOG_ERROR << "from field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Address> to;
        if (input.hasKey("to")) {
            to = deserializeAddress(input.get<std::string>("to"));
        }
        else {
            LOG_ERROR << "to field is not exists";
            return std::nullopt;
        }
        std::optional<base::Time> timestamp;
        if (input.hasKey("timestamp")) {
            timestamp = base::Time(input.get<std::uint_least32_t>("timestamp"));
        }
        else {
            LOG_ERROR << "timestamp field is not exists";
            return std::nullopt;
        }
        std::optional<base::Bytes> data;
        if (input.hasKey("data")) {
            data = deserializeBytes(input.get<std::string>("data"));
        }
        else {
            LOG_ERROR << "data field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Sign> sign;
        if (input.hasKey("sign")) {
            sign = deserializeSign(input.get<std::string>("sign"));
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


base::PropertyTree serializeBlock(const lk::ImmutableBlock& block)
{
    base::PropertyTree result;
    result.add("depth", block.getDepth());
    result.add("nonce", block.getNonce());
    result.add("coinbase", serializeAddress(block.getCoinbase()));
    result.add("previous_block_hash", serializeHash(block.getPrevBlockHash()));
    result.add("timestamp", block.getTimestamp().getSeconds());

    base::PropertyTree txs_values;
    for (auto& tx : block.getTransactions()) {
        txs_values.add("", serializeTransaction(tx));
    }
    result.add("transactions", std::move(txs_values));
    return result;
}


std::optional<lk::ImmutableBlock> deserializeBlock(const base::PropertyTree& input)
{
    try {
        std::optional<std::uint64_t> depth;
        if (input.hasKey("depth")) {
            depth = input.get<std::uint64_t>("depth");
        }
        else {
            LOG_ERROR << "depth field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> nonce;
        if (input.hasKey("nonce")) {
            nonce = input.get<std::uint64_t>("nonce");
        }
        else {
            LOG_ERROR << "nonce field is not exists";
            return std::nullopt;
        }
        std::optional<base::Time> timestamp;
        if (input.hasKey("timestamp")) {
            timestamp = base::Time(input.get<std::uint_least32_t>("timestamp"));
        }
        else {
            LOG_ERROR << "timestamp field is not exists";
            return std::nullopt;
        }
        std::optional<base::Sha256> previous_block_hash;
        if (input.hasKey("previous_block_hash")) {
            previous_block_hash = deserializeHash(input.get<std::string>("previous_block_hash"));
        }
        else {
            LOG_ERROR << "previous_block_hash field is not exists";
            return std::nullopt;
        }
        std::optional<lk::Address> coinbase;
        if (input.hasKey("coinbase")) {
            coinbase = deserializeAddress(input.get<std::string>("coinbase"));
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
        if (input.hasKey("transactions")) {
            for (const auto& res_tx : input.getSubTree("transactions")) {
                base::PropertyTree sub(res_tx.second);
                auto tx_opt = deserializeTransaction(sub);
                if (tx_opt) {
                    txs.add(*tx_opt);
                }
                else {
                    LOG_ERROR << "error at hash deserialization";
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


base::PropertyTree serializeTransactionStatus(const lk::TransactionStatus& status)
{
    base::PropertyTree result;
    result.add("status_code", serializeTransactionStatusStatusCode(status.getStatus()));
    result.add("action_type", serializeTransactionStatusActionType(status.getType()));
    result.add("fee_left", serializeFee(status.getFeeLeft()));
    result.add("message", status.getMessage());
    return result;
}


std::optional<lk::TransactionStatus> deserializeTransactionStatus(const base::PropertyTree& input)
{
    try {
        std::optional<lk::TransactionStatus::StatusCode> status_code;
        if (input.hasKey("status_code")) {
            status_code = deserializeTransactionStatusStatusCode(input.get<std::uint32_t>("status_code"));
        }
        else {
            LOG_ERROR << "status_code field is not exists";
            return std::nullopt;
        }
        std::optional<lk::TransactionStatus::ActionType> action_type;
        if (input.hasKey("action_type")) {
            action_type = deserializeTransactionStatusActionType(input.get<std::uint32_t>("action_type"));
        }
        else {
            LOG_ERROR << "action_type field is not exists";
            return std::nullopt;
        }
        std::optional<std::uint64_t> fee;
        if (input.hasKey("fee_left")) {
            fee = deserializeFee(input.get<std::string>("fee_left"));
        }
        else {
            LOG_ERROR << "fee_left field is not exists";
            return std::nullopt;
        }
        std::optional<std::string> message;
        if (input.hasKey("message")) {
            message = input.get<std::string>("message");
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