#include "base_rpc.hpp"

namespace rpc
{

TransactionStatus::TransactionStatus(StatusCode status, std::string message, lk::Balance gas_left) noexcept
  : _status{ status }
  , _message{ message }
  , _gas_left{ gas_left }
{}

TransactionStatus TransactionStatus::createSuccess(lk::Balance gas_left, const std::string& message) noexcept
{
    return TransactionStatus(TransactionStatus::StatusCode::Success, message, gas_left);
}

TransactionStatus TransactionStatus::createRejected(lk::Balance gas_left, const std::string& message) noexcept
{
    return TransactionStatus(TransactionStatus::StatusCode::Rejected, message, gas_left);
}

TransactionStatus TransactionStatus::createRevert(lk::Balance gas_left, const std::string& message) noexcept
{
    return TransactionStatus(TransactionStatus::StatusCode::Revert, message, gas_left);
}

TransactionStatus TransactionStatus::createFailed(lk::Balance gas_left, const std::string& message) noexcept
{
    return TransactionStatus(TransactionStatus::StatusCode::Failed, message, gas_left);
}

TransactionStatus::operator bool() const noexcept
{
    return _status == TransactionStatus::StatusCode::Success;
}

bool TransactionStatus::operator!() const noexcept
{
    return _status != TransactionStatus::StatusCode::Success;
}

const std::string& TransactionStatus::getMessage() const noexcept
{
    return _message;
}

std::string& TransactionStatus::getMessage() noexcept
{
    return _message;
}

TransactionStatus::StatusCode TransactionStatus::getStatus() const noexcept
{
    return _status;
}

lk::Balance TransactionStatus::getGasLeft() const noexcept
{
    return _gas_left;
}

} // namespace rpc