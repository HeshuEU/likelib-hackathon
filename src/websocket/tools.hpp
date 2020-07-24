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

std::string serializeCommandName(websocket::Command::Id name);

websocket::Command::Name deserializeCommandName(const std::string& message);

std::string serializeCommandType(websocket::Command::Id command_type);

websocket::Command::Type deserializeCommandType(const std::string& message);

std::string serializeAccountType(lk::AccountType type);

lk::AccountType deserializeAccountType(const std::string& type);

uint32_t serializeTransactionStatusStatusCode(lk::TransactionStatus::StatusCode status_code);

lk::TransactionStatus::StatusCode deserializeTransactionStatusStatusCode(std::uint32_t type);

uint32_t serializeTransactionStatusActionType(lk::TransactionStatus::ActionType action_type);

lk::TransactionStatus::ActionType deserializeTransactionStatusActionType(std::uint32_t type);

std::string serializeBalance(const lk::Balance& balance);

lk::Balance deserializeBalance(const std::string& type);

std::string serializeFee(lk::Fee balance);

lk::Fee deserializeFee(const std::string& type);

std::string serializeHash(const base::Sha256& hash);

base::Sha256 deserializeHash(const std::string& type);

std::string serializeAddress(const lk::Address& address);

lk::Address deserializeAddress(const std::string& type);

std::string serializeBytes(const base::Bytes& data);

base::Bytes deserializeBytes(const std::string& data);

std::string serializeSign(const lk::Sign& sign);

lk::Sign deserializeSign(const std::string& data);

rapidjson::Value serializeAccountInfo(const lk::AccountInfo& account_info,
                                      rapidjson::Document::AllocatorType& allocator);

lk::AccountInfo deserializeAccountInfo(rapidjson::Value input);

rapidjson::Value serializeInfo(const NodeInfo& info, rapidjson::Document::AllocatorType& allocator);

NodeInfo deserializeInfo(rapidjson::Value input);

rapidjson::Value serializeTransaction(const lk::Transaction& tx, rapidjson::Document::AllocatorType& allocator);

lk::Transaction deserializeTransaction(rapidjson::Value input);

rapidjson::Value serializeBlock(const lk::ImmutableBlock& block, rapidjson::Document::AllocatorType& allocator);

lk::ImmutableBlock deserializeBlock(rapidjson::Value input);

rapidjson::Value serializeTransactionStatus(const lk::TransactionStatus& status,
                                            rapidjson::Document::AllocatorType& allocator);

lk::TransactionStatus deserializeTransactionStatus(rapidjson::Value input);

}
