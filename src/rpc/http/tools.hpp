#pragma once

#include "rpc/base_rpc.hpp"

#include <cpprest/http_client.h>

namespace rpc::http
{

web::json::value serializeAccountType(lk::AccountType type);

std::optional<lk::AccountType> deserializeAccountType(const std::string& type);

web::json::value serializeTransactionStatusStatusCode(lk::TransactionStatus::StatusCode status_code);

std::optional<lk::TransactionStatus::StatusCode> deserializeTransactionStatusStatusCode(std::uint32_t type);

web::json::value serializeTransactionStatusActionType(lk::TransactionStatus::ActionType action_type);

std::optional<lk::TransactionStatus::ActionType> deserializeTransactionStatusActionType(std::uint32_t type);

web::json::value serializeBalance(const lk::Balance& balance);

std::optional<lk::Balance> deserializeBalance(const std::string& type);

web::json::value serializeFee(std::uint64_t balance);

std::optional<std::uint64_t> deserializeFee(const std::string& type);

web::json::value serializeHash(const base::Sha256& hash);

std::optional<base::Sha256> deserializeHash(const std::string& type);

web::json::value serializeAddress(const lk::Address& address);

std::optional<lk::Address> deserializeAddress(const std::string& type);

web::json::value serializeBytes(const base::Bytes& data);

std::optional<base::Bytes> deserializeBytes(const std::string& data);

web::json::value serializeSign(const lk::Sign& sign);

std::optional<lk::Sign> deserializeSign(const std::string& data);

web::json::value serializeAccountInfo(const lk::AccountInfo& account_info);

std::optional<lk::AccountInfo> deserializeAccountInfo(const web::json::value& input);

web::json::value serializeInfo(const Info& info);

std::optional<Info> deserializeInfo(const web::json::value& input);

web::json::value serializeTransaction(const lk::Transaction& tx);

std::optional<lk::Transaction> deserializeTransaction(const web::json::value& input);

web::json::value serializeBlock(const lk::ImmutableBlock& block);

std::optional<lk::ImmutableBlock> deserializeBlock(const web::json::value& input);

web::json::value serializeTransactionStatus(const lk::TransactionStatus& status);

std::optional<lk::TransactionStatus> deserializeTransactionStatus(const web::json::value& input);

}