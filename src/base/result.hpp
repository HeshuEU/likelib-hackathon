#pragma once

#include "error.hpp"

#include <optional>
#include <string>
#include <variant>

namespace base
{

template<typename T>
class Result
{
  public:
    template<typename... Args>
    static Result<T> ok(Args&&... args);

    static Result<T> fail(const Error&);

    static Result<T> fail(Error&&);

    //----------------
    Result<T>(const T&);

    Result<T>(T&&);

    // template<typename... Args>
    // Result<T>(Args&&... args);


    Result<T>(const base::Error& error);

    Result<T>(base::Error&& error);

    Result<T>(const Result<T>&) = default;

    Result<T>& operator=(const Result<T>&) = default;

    Result(Result<T>&&) = default;

    Result& operator=(Result<T>&&) = default;

    ~Result<T>() = default;
    //----------------

    bool isOk() const noexcept;

    bool isError() const noexcept;

    operator bool() const noexcept;

    //----------------

    const base::Error& getError() const&;

    base::Error&& getError() &&;

    //----------------

    const T& getValue() const&;

    T&& getValue() &&;

    //----------------

    operator T() const;

    std::string toString() const;

  private:
    std::variant<T, base::Error> _value;
};

} // namespace base

#define RETURN_IF_ERROR_RESULT(result) \
    if(!(result)) \
    return static_cast<decltype(result)&&>(result).getError()

#include "result.tpp"
