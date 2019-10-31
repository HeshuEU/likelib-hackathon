#include "error.hpp"

#include <ostream>

namespace
{

std::string buildMessage(const base::ErrorCode& ec, const std::string& message)
{
    std::string ret;
    if(ec != base::ErrorCode::NONE) {
        ret += base::EnumToString(ec);
    }
    if(!message.empty()) {
        if(!ret.empty()) {
            ret += " ";
        }
        ret += message;
    }
    return ret;
}

} // namespace


namespace base
{

Error::Error(const std::string& message) : _error_code{ErrorCode::NONE}, _message{buildMessage(_error_code, message)}
{}


Error::Error(const ErrorCode& ec, const std::string& message)
    : _error_code{ec}, _message{buildMessage(_error_code, message)}
{}


const std::string& Error::toStdString() const noexcept
{
    return _message;
}


const char* Error::what() const noexcept
{
    return _message.c_str();
}


ErrorCode Error::getErrorCode() const noexcept
{
    return _error_code;
}

std::ostream& operator<<(std::ostream& os, const Error& error)
{
    return os << error.what();
}

} // namespace base