#pragma once

#include "base/error.hpp"

namespace base
{

class Status
{
  public:
    //=====================
    static Status Ok();

    static Status Error(const base::Error& error);

    template<typename... Args>
    static Status Error(Args&&... args);

    //=====================
    Status(const Status&) = default;
    Status& operator=(const Status&) = default;

    Status(Status&&) = default;
    Status& operator=(Status&&) = default;

    ~Status() = default;
    //=====================

    bool isOk() const noexcept;
    bool isError() const noexcept;

    explicit operator bool() const noexcept;

    // Result<Error> getError() const noexcept;
  private:
    //=====================
    Status();
    Status(const base::Error& error);
    //=====================
    bool _is_ok;
    base::Error _error;
};

} // namespace base

#define RETURN_IF_ERROR_STATUS(status) if(!(status)) return (status).getError()


#include "status.tpp"