#pragma once

#include <core/address.hpp>
#include <core/block.hpp>
#include <core/managers.hpp>
#include <core/transaction.hpp>
#include <core/types.hpp>

#include <base/time.hpp>

#include <string>
#include <tuple>

namespace rpc
{

class TransactionStatus
{
  public:
    enum StatusCode : uint8_t
    {
        Success = 1,
        Rejected = 2,
        Revert = 3,
        Failed = 4
    };

    explicit TransactionStatus(StatusCode status, std::string message, lk::Balance gas_left) noexcept;

    TransactionStatus(const TransactionStatus&) = default;
    TransactionStatus(TransactionStatus&&) = default;
    TransactionStatus& operator=(const TransactionStatus&) = default;
    TransactionStatus& operator=(TransactionStatus&&) = default;

    static TransactionStatus createSuccess(lk::Balance gas_left, const std::string& message = "") noexcept;
    static TransactionStatus createRejected(lk::Balance gas_left = 0, const std::string& message = "") noexcept;
    static TransactionStatus createRevert(lk::Balance gas_left = 0, const std::string& message = "") noexcept;
    static TransactionStatus createFailed(lk::Balance gas_left = 0, const std::string& message = "") noexcept;

    operator bool() const noexcept;

    bool operator!() const noexcept;

    const std::string& getMessage() const noexcept;
    std::string& getMessage() noexcept;

    StatusCode getStatus() const noexcept;

    lk::Balance getGasLeft() const noexcept;

  private:
    StatusCode _status;
    std::string _message;
    lk::Balance _gas_left;
};


struct Info
{
    base::Sha256 top_block_hash;
    uint32_t api_version;
    std::size_t peers_number;
};


class BaseRpc
{
  public:
    virtual ~BaseRpc() = default;

    virtual lk::AccountInfo getAccount(const lk::Address& address) = 0;

    virtual Info getNodeInfo() = 0;

    virtual lk::Block getBlock(const base::Sha256& block_hash) = 0;

    virtual lk::Transaction getTransaction(const base::Sha256& transaction_hash) = 0;

    virtual TransactionStatus pushTransaction(const lk::Transaction& transaction) = 0;

    virtual TransactionStatus getTransactionResult(const base::Sha256& transaction_hash) = 0;
};

} // namespace rpc
