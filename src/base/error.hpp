#pragma once

#include "base/stringifiable_enum_class.hpp"

#include <boost/current_function.hpp>

#include <exception>
#include <iosfwd>
#include <string>

namespace base
{
class Error : public std::exception
{
  public:
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

class UnaccessibleFile : public Error
{
    using Error::Error;
};

class SystemCallFailed : public Error
{
    using Error::Error;
};

std::ostream& operator<<(std::ostream& os, const Error& error);

#define RAISE_ERROR(message) \
    throw base::Error(std::string{__FILE__} + std::string{":"} + std::to_string(__LINE__) + std::string{" :: "} + \
                      std::string{BOOST_CURRENT_FUNCTION} + std::string{" :: "} + (message))


#define CLARIFY_ERROR(expr, message) \
    try { \
        expr; \
    } \
    catch(const std::exception& e) { \
        throw base::Error{std::string{message} + std::string{": "} + e.what()}; \
    }

} // namespace base
