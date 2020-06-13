#pragma once

#include <boost/current_function.hpp>

#include <exception>
#include <string>

namespace base
{

class Error : public std::exception
{
  public:
    Error(const char* file_name, std::size_t line_number, const char* function_signature, std::string message = {});
    Error(const Error&) = default;
    Error(Error&&) = default;
    ~Error() override = default;
    Error& operator=(const Error&) = default;
    Error& operator=(Error&&) = default;
    const std::string& getMessage() const noexcept;
    const char* what() const noexcept override;

  private:
    const char* _file_name;
    std::size_t _line_number;
    const char* _function_signature;
    std::string _message;
    std::string _full_message;
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

class UseOfUninitializedValue : public RuntimeError
{
    using RuntimeError::RuntimeError;
};

#define RAISE_ERROR(error_type, ...) throw error_type(__FILE__, __LINE__, BOOST_CURRENT_FUNCTION, __VA_ARGS__)

} // namespace base
