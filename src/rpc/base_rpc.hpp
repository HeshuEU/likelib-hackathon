#pragma once

#include "base/time.hpp"
#include "bc/address.hpp"
#include "bc/transaction.hpp"
#include "bc/types.hpp"

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

class BaseRpc
{
  public:
    virtual ~BaseRpc() = default;

    /// method call remote server method(specified ip address in constructor) with similar params
    /// \param address of account
    /// \return result of balance by specific address
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    virtual bc::Balance balance(const bc::Address& address) = 0;

    virtual std::tuple<OperationStatus, bc::Address, bc::Balance> transaction_creation_contract(bc::Balance amount,
        const bc::Address& from_address, const base::Time& transaction_time,
        bc::Balance gas, const base::Bytes& code, const base::Bytes& initial_message, const bc::Sign& signature) = 0;

    virtual std::tuple<OperationStatus, base::Bytes, bc::Balance> transaction_to_contract(bc::Balance amount,
        const bc::Address& from_address, const bc::Address& to_address, const base::Time& transaction_time,
        bc::Balance gas, const base::Bytes& message, const bc::Sign& signature) = 0;

    virtual OperationStatus transaction_to_wallet(bc::Balance amount, const bc::Address& from_address,
        const bc::Address& to_address, bc::Balance fee, const base::Time& transaction_time, const bc::Sign& signature) = 0;

    /// method call remote server method(specified ip address in constructor) with similar params
    /// \param test_request sha256 from secret data request
    /// \return sha256 from secret data response
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    virtual OperationStatus test(uint32_t api_version) = 0;
};

} // namespace rpc
