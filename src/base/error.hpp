#pragma once

#include "base/stringifiable_enum_class.hpp"

#include <string>

namespace base
{
DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(
    ErrorCode, (NONE)(INVALID_PARAMETER)(FILE_NOT_FOUND)(SYSTEM_CALL_FAILED)(VALUE_TO_ERROR)(ERROR_TO_VALUE));

class Error
{
  public:
    Error(const std::string& message);
    Error(const ErrorCode& ec, const std::string& message = {});
    Error(const Error&) = default;
    Error(Error&&) = default;
    ~Error() = default;

    Error& operator=(const Error&) = default;
    Error& operator=(Error&&) = default;

    const std::string& what() const noexcept;

    ErrorCode getErrorCode() const noexcept;

  private:
    ErrorCode _error_code;
    std::string _message;
};
} // namespace base
