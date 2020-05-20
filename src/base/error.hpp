#pragma once

#include <boost/current_function.hpp>

#include <exception>
#include <iosfwd>
#include <string>

namespace base
{
class Error : public std::exception
{
  public:
    Error() = default;
    Error(const std::string& message);
    Error(const Error&) = default;
    Error(Error&&) = default;
    ~Error() = default;
    Error& operator=(const Error&) = default;
    Error& operator=(Error&&) = default;
    const std::string& toStdString() const noexcept;
    const char* what() const noexcept override;
  private:
    std::string _message;
};

class InvalidArgument : public Error
{
    using Error::Error;
};

class InaccessibleFile : public Error
{
    using Error::Error;
};

class SystemCallFailed : public Error
{
    using Error::Error;
};

class ParsingError : public Error
{
    using Error::Error;
};

class DatabaseError : public Error
{
    using Error::Error;
};

class CryptoError : public Error
{
    using Error::Error;
};

class LogicError : public Error
{
    using Error::Error;
};

class RuntimeError : public Error
{
    using Error::Error;
};

class ValueNotFound : public RuntimeError
{
    using RuntimeError::RuntimeError;
};


std::ostream& operator<<(std::ostream& os, const Error& error);

#define _RAISE_ERROR1(error_type)                                                                                        \
    throw error_type(std::string{ __FILE__ } + std::string{ ":" } + std::to_string(__LINE__) + std::string{ " :: " } + \
                     std::string{ BOOST_CURRENT_FUNCTION })

#define _RAISE_ERROR2(error_type, message)                                                                               \
    throw error_type(std::string{ __FILE__ } + std::string{ ":" } + std::to_string(__LINE__) + std::string{ " :: " } + \
                     std::string{ BOOST_CURRENT_FUNCTION } + std::string{ " :: " } + (message))

#define GET_RAISE_ERROR(_1, _2, _3, NAME, ...) NAME
#define RAISE_ERROR(...) GET_RAISE_ERROR(__VA_ARGS__, _RAISE_ERROR2, _RAISE_ERROR1)(__VA_ARGS__)


#define CLARIFY_ERROR(error_type, expr, message)                                                                       \
    try {                                                                                                              \
        expr;                                                                                                          \
    }                                                                                                                  \
    catch (const std::exception& e) {                                                                                  \
        RAISE_ERROR(error_type, std::string{ message } + std::string{ ": " } + e.what());                              \
    }

} // namespace base
