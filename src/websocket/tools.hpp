#pragma once

#include "types.hpp"

#include "core/block.hpp"
#include "core/core.hpp"
#include "core/managers.hpp"
#include "core/transaction.hpp"

#include <boost/beast/core.hpp>

namespace websocket
{

boost::asio::ip::tcp::endpoint createEndpoint(const std::string& listening_address);

base::json::Value serializeCommandName(websocket::Command::Id name);

websocket::Command::Name deserializeCommandName(const std::string& message);

base::json::Value serializeCommandType(websocket::Command::Id command_type);

websocket::Command::Type deserializeCommandType(const std::string& message);

base::json::Value serializeAccountType(lk::AccountType type);

lk::AccountType deserializeAccountType(const std::string& type);

base::json::Value serializeTransactionStatusStatusCode(lk::TransactionStatus::StatusCode status_code);

lk::TransactionStatus::StatusCode deserializeTransactionStatusStatusCode(std::uint32_t type);

base::json::Value serializeTransactionStatusActionType(lk::TransactionStatus::ActionType action_type);

lk::TransactionStatus::ActionType deserializeTransactionStatusActionType(std::uint32_t type);

base::json::Value serializeBalance(const lk::Balance& balance);

lk::Balance deserializeBalance(const std::string& type);

base::json::Value serializeFee(lk::Fee balance);

lk::Fee deserializeFee(const std::string& type);

base::json::Value serializeHash(const base::Sha256& hash);

base::Sha256 deserializeHash(const std::string& type);

base::json::Value serializeAddress(const lk::Address& address);

lk::Address deserializeAddress(const std::string& type);

base::json::Value serializeBytes(const base::Bytes& data);

base::Bytes deserializeBytes(const std::string& data);

base::json::Value serializeSign(const lk::Sign& sign);

lk::Sign deserializeSign(const std::string& data);

base::json::Value serializeAccountInfo(const lk::AccountInfo& account_info);

lk::AccountInfo deserializeAccountInfo(base::json::Value input);

base::json::Value serializeInfo(const NodeInfo& info);

NodeInfo deserializeInfo(base::json::Value input);

base::json::Value serializeTransaction(const lk::Transaction& tx);

lk::Transaction deserializeTransaction(base::json::Value input);

base::json::Value serializeBlock(const lk::ImmutableBlock& block);

lk::ImmutableBlock deserializeBlock(base::json::Value input);

base::json::Value serializeTransactionStatus(const lk::TransactionStatus& status);

lk::TransactionStatus deserializeTransactionStatus(base::json::Value input);

}
