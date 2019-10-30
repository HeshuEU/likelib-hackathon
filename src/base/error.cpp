#include "error.hpp"

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


const std::string& Error::what() const noexcept
{
    return _message;
}


ErrorCode Error::getErrorCode() const noexcept
{
    return _error_code;
}

}