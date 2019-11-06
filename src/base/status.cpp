#include "status.hpp"

namespace base
{

Status Status::Ok()
{
    return Status{};
}


Status Status::Error(const base::Error& error)
{
    return Status{error};
}


Status::Status() : _is_ok{true}, _error{base::StatusCode::NONE}
{}


Status::Status(const base::Error& error) : _is_ok{false}, _error{error}
{}


bool Status::isOk() const noexcept
{
    return _is_ok;
}

bool Status::isError() const noexcept
{
    return !_is_ok;
}


Status::operator bool() const noexcept
{
    return _is_ok;
}


} // namespace base