#include "http_client.hpp"

#include <rpc/error.hpp>

namespace rpc::http
{

NodeClient::NodeClient(const std::string& connect_address) {}


uint32_t NodeClient::get_api_version() {}


lk::Balance NodeClient::balance(const lk::Address& address) {}


Info NodeClient::info() {}


lk::Block NodeClient::get_block(const base::Sha256& block_hash) {}


std::tuple<OperationStatus, lk::Address, lk::Balance> NodeClient::transaction_create_contract(
  lk::Balance amount,
  const lk::Address& from_address,
  const base::Time& transaction_time,
  lk::Balance gas,
  const std::string& contract_code,
  const std::string& init,
  const lk::Sign& signature)
{}


std::tuple<OperationStatus, std::string, lk::Balance> NodeClient::transaction_message_call(
  lk::Balance amount,
  const lk::Address& from_address,
  const lk::Address& to_address,
  const base::Time& transaction_time,
  lk::Balance fee,
  const std::string& data,
  const lk::Sign& signature)
{}

} // namespace rpc
