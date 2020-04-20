#pragma once

#include <core/transaction.hpp>

#include <rpc/base_rpc.hpp>

#include <public_rpc.grpc.pb.h>

namespace rpc::grpc
{

void serializeTransaction(const lk::Transaction& from, likelib::Transaction& to);

lk::Transaction&& deserializeTransaction(const ::likelib::Transaction& serialized_tx);

void serializeTransactionStatus(const TransactionStatus& from, likelib::TransactionStatus& to);

TransactionStatus deserializeTransactionStatus(const likelib::TransactionStatus& status);

}