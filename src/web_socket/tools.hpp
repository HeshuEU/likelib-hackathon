#pragma once

#include "core/block.hpp"
#include "core/core.hpp"
#include "core/managers.hpp"
#include "core/transaction.hpp"

#include "base/property_tree.hpp"

#include <boost/beast/core.hpp>

namespace web_socket
{

struct NodeInfo
{
    base::Sha256 top_block_hash;
    uint64_t top_block_number;
};

boost::asio::ip::tcp::endpoint create_endpoint(const std::string& listening_address);

const std::string& serializeAccountType(lk::AccountType type);

std::optional<lk::AccountType> deserializeAccountType(const std::string& type);

uint32_t serializeTransactionStatusStatusCode(lk::TransactionStatus::StatusCode status_code);

std::optional<lk::TransactionStatus::StatusCode> deserializeTransactionStatusStatusCode(std::uint32_t type);

uint32_t serializeTransactionStatusActionType(lk::TransactionStatus::ActionType action_type);

std::optional<lk::TransactionStatus::ActionType> deserializeTransactionStatusActionType(std::uint32_t type);

std::string serializeBalance(const lk::Balance& balance);

std::optional<lk::Balance> deserializeBalance(const std::string& type);

std::string serializeFee(std::uint64_t balance);

std::optional<lk::Fee> deserializeFee(const std::string& type);

std::string serializeHash(const base::Sha256& hash);

std::optional<base::Sha256> deserializeHash(const std::string& type);

std::string serializeAddress(const lk::Address& address);

std::optional<lk::Address> deserializeAddress(const std::string& type);

std::string serializeBytes(const base::Bytes& data);

std::optional<base::Bytes> deserializeBytes(const std::string& data);

std::string serializeSign(const lk::Sign& sign);

std::optional<lk::Sign> deserializeSign(const std::string& data);

base::PropertyTree serializeAccountInfo(const lk::AccountInfo& account_info);

std::optional<lk::AccountInfo> deserializeAccountInfo(const base::PropertyTree& input);

base::PropertyTree serializeInfo(const NodeInfo& info);

std::optional<NodeInfo> deserializeInfo(const base::PropertyTree& input);

base::PropertyTree serializeTransaction(const lk::Transaction& tx);

std::optional<lk::Transaction> deserializeTransaction(const base::PropertyTree& input);

base::PropertyTree serializeBlock(const lk::Block& block);

std::optional<lk::Block> deserializeBlock(const base::PropertyTree& input);

base::PropertyTree serializeTransactionStatus(const lk::TransactionStatus& status);

std::optional<lk::TransactionStatus> deserializeTransactionStatus(const base::PropertyTree& input);

base::PropertyTree serializeViewCall(const lk::ViewCall& call);

std::optional<lk::ViewCall> deserializeViewCall(const base::PropertyTree& input);

}
