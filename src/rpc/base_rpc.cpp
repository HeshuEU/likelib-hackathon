#include "base_rpc.hpp"

namespace rpc
{

OperationStatus::OperationStatus(StatusCode status, std::string message) noexcept : _status{status}, _message{message}
{}

OperationStatus OperationStatus::createSuccess(const std::string& message) noexcept
{
    return OperationStatus(OperationStatus::StatusCode::Success, message);
}

OperationStatus OperationStatus::createRejected(const std::string& message) noexcept
{
    return OperationStatus(OperationStatus::StatusCode::Rejected, message);
}

OperationStatus OperationStatus::createFailed(const std::string& message) noexcept
{
    return OperationStatus(OperationStatus::StatusCode::Failed, message);
}

OperationStatus::operator bool() const noexcept
{
    return _status == OperationStatus::StatusCode::Success;
}

bool OperationStatus::operator!() const noexcept
{
    return _status != OperationStatus::StatusCode::Success;
}

const std::string& OperationStatus::getMessage() const noexcept
{
    return _message;
}

std::string& OperationStatus::getMessage() noexcept
{
    return _message;
}

OperationStatus::StatusCode OperationStatus::getStatus() const noexcept
{
    return _status;
}
} // namespace rpc