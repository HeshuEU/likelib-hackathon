#pragma once

#include "base/stringifiable_enum_class.hpp"

#include <boost/current_function.hpp>

#include <exception>
#include <iosfwd>
#include <string>

namespace base
{
DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    ErrorCode,
    (NONE)(INVALID_ARGUMENT)(FILE_NOT_FOUND)(SYSTEM_CALL_FAILED)(VALUE_TO_ERROR)(ERROR_TO_VALUE)(FUNCTION_CALL_FAILED));

class Error : public std::exception
{
  public:
    Error(const std::string& message);

    Error(const ErrorCode& ec, const std::string& message = {});

    Error(const Error&) = default;

    Error(Error&&) = default;

    ~Error() = default;

    Error& operator=(const Error&) = default;

    Error& operator=(Error&&) = default;

    const std::string& toStdString() const noexcept;

    const char* what() const noexcept override;

    ErrorCode getErrorCode() const noexcept;

  private:
    ErrorCode _error_code;
    std::string _message;
};

std::ostream& operator<<(std::ostream& os, const Error& error);

#define RAISE_ERROR(message) \
    throw base::Error(std::string{__FILE__} + std::string{":"} + std::string{__LINE__} + \
                      std::string{BOOST_CURRENT_FUNCTION} + std::string{" ::"} + (message))

} // namespace base
