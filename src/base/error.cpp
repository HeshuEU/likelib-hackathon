#include "error.hpp"

#include <ostream>

namespace base
{

Error::Error(const std::string& message) : _message{message}
{}


const std::string& Error::toStdString() const noexcept
{
    return _message;
}


const char* Error::what() const noexcept
{
    return _message.c_str();
}


std::ostream& operator<<(std::ostream& os, const Error& error)
{
    return os << error.what();
}

} // namespace base