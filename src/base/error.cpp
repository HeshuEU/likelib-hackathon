#include "error.hpp"

#include <ostream>

namespace
{

std::string buildMessage(const base::StatusCode& ec, const std::string& message)
{
    std::string ret;
    if(ec != base::StatusCode::NONE) {
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

Error::Error(const std::string& message) : _error_code{StatusCode::NONE}, _message{buildMessage(_error_code, message)}
{}


Error::Error(const StatusCode& ec, const std::string& message)
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


StatusCode Error::getStatusCode() const noexcept
{
    return _error_code;
}

std::ostream& operator<<(std::ostream& os, const Error& error)
{
    return os << error.what();
}

} // namespace base