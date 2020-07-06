#pragma once

#include <public_rpc.grpc.pb.h>

#include "rpc/base_rpc.hpp"

namespace rpc::grpc
{

likelib::AccountInfo_Type serializeAccountType(lk::AccountType type);

lk::AccountType deserializeAccountType(likelib::AccountInfo_Type type);

likelib::TransactionStatus_StatusCode serializeTransactionStatusCode(lk::TransactionStatus::StatusCode status_code);

lk::TransactionStatus::StatusCode deserializeTransactionStatusCode(likelib::TransactionStatus_StatusCode status_code);

likelib::TransactionStatus_ActionType serializeTransactionActionType(lk::TransactionStatus::ActionType action_type);

lk::TransactionStatus::ActionType deserializeTransactionActionType(likelib::TransactionStatus_ActionType action_type);

void serializeAddress(const lk::Address& from, likelib::Address* to);

lk::Address deserializeAddress(const likelib::Address* const address);

void serializeAccountInfo(const lk::AccountInfo& from, likelib::AccountInfo* to);

lk::AccountInfo deserializeAccountInfo(const likelib::AccountInfo* const info);

void serializeHash(const base::Sha256& from, likelib::Hash* to);

base::Sha256 deserializeHash(const ::likelib::Hash* const hash);

void serializeInfo(const Info& from, likelib::NodeInfo* to);

Info deserializeInfo(const likelib::NodeInfo* const info);

void serializeNumber(uint64_t from, likelib::Number* to);

base::Bytes deserializeData(const likelib::Data* const data);

void serializeTransaction(const lk::Transaction& from, likelib::Transaction* to);

lk::Transaction deserializeTransaction(const ::likelib::Transaction* const tx);

void serializeBlock(const lk::ImmutableBlock& from, likelib::Block* to);

lk::ImmutableBlock deserializeBlock(const likelib::Block* const block);

void serializeTransactionStatus(const lk::TransactionStatus& from, likelib::TransactionStatus* to);

lk::TransactionStatus deserializeTransactionStatus(const likelib::TransactionStatus* const status);

}