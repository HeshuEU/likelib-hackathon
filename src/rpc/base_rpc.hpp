#pragma once

#include "base/time.hpp"
#include "core/address.hpp"
#include "core/block.hpp"
#include "core/transaction.hpp"
#include "core/types.hpp"

#include <string>
#include <tuple>

namespace rpc
{

class OperationStatus
{
  public:
    enum StatusCode
    {
        Success = 1,
        Rejected = 2,
        Failed = 3
    };
    OperationStatus() = delete;

    OperationStatus(StatusCode status, std::string message) noexcept;

    OperationStatus(const OperationStatus&) = default;
    OperationStatus(OperationStatus&&) = default;
    OperationStatus& operator=(const OperationStatus&) = default;
    OperationStatus& operator=(OperationStatus&&) = default;

    static OperationStatus createSuccess(const std::string& message = "") noexcept;
    static OperationStatus createRejected(const std::string& message = "") noexcept;
    static OperationStatus createFailed(const std::string& message = "") noexcept;

    operator bool() const noexcept;

    bool operator!() const noexcept;

    const std::string& getMessage() const noexcept;
    std::string& getMessage() noexcept;

    StatusCode getStatus() const noexcept;

  private:
    StatusCode _status;
    std::string _message;
};


struct Info
{
    base::Sha256 top_block_hash;
    std::size_t peers_number;
};


class BaseRpc
{
  public:
    virtual ~BaseRpc() = default;

    /// method call remote server method(specified ip address in constructor) with similar params
    /// \param address of account
    /// \return result of balance by specific address
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    virtual lk::Balance balance(const lk::Address& address) = 0;

    /// method call remote server method(specified ip address in constructor) with similar params
    /// \param test_request sha256 from secret data request
    /// \return sha256 from secret data response
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    virtual OperationStatus test(uint32_t api_version) = 0;

    virtual Info info() = 0;

    virtual lk::Block get_block(const base::Sha256& block_hash) = 0;

    virtual std::tuple<OperationStatus, lk::Address, lk::Balance> transaction_create_contract(
      lk::Balance amount,
      const lk::Address& from_address,
      const base::Time& transaction_time,
      lk::Balance gas,
      const std::string& contract_code,
      const std::string& init,
      const lk::Sign& signature) = 0;

    virtual std::tuple<OperationStatus, std::string, lk::Balance> transaction_message_call(
      lk::Balance amount,
      const lk::Address& from_address,
      const lk::Address& to_address,
      const base::Time& transaction_time,
      lk::Balance gas,
      const std::string& message,
      const lk::Sign& signature) = 0;
};

} // namespace rpc
