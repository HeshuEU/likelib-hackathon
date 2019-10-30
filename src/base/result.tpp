#pragma once

#include "result.hpp"

#include "assert.hpp"

#include <utility>


namespace base
{


template<typename T>
template<typename... Args>
Result<T> Result<T>::ok(Args&&... args)
{
    return Result<T>{std::forward<Args>(args)...};
}


template<typename T>
Result<T> Result<T>::fail(const Error& error)
{
    return Result<T>{error};
}


template<typename T>
Result<T> Result<T>::fail(Error&& error)
{
    return Result<T>{std::move(error)};
}


template<typename T>
Result<T>::Result(const base::Error& error) : _value(std::in_place_index<1>, error)
{}


template<typename T>
Result<T>::Result(base::Error&& error) : _value(std::in_place_index<1>, std::move(error))
{}


template<typename T>
Result<T>::Result(const T& v) : _value(std::in_place_index<0>, v)
{}


template<typename T>
Result<T>::Result(T&& v) : _value(std::in_place_index<0>, std::move(v))
{}


template<typename T>
bool Result<T>::isOk() const noexcept
{
    return _value.index() == 0;
}


template<typename T>
bool Result<T>::isError() const noexcept
{
    return _value.index() == 1;
}


template<typename T>
Result<T>::operator bool() const noexcept
{
    return isOk();
}


template<typename T>
std::string Result<T>::toString() const
{
    if(isOk()) {
        return std::to_string(std::get<0>(_value));
    }
    else {
        return std::get<1>(_value).what();
    }
}


template<typename T>
Result<T>::operator T() const
{
    return getValue();
}


template<typename T>
const base::Error& Result<T>::getError() const&
{
    CHECK(isError(), base::EnumToString(base::ErrorCode::VALUE_TO_ERROR));

    return std::get<1>(_value);
}


template<typename T>
base::Error&& Result<T>::getError() &&
{
    CHECK(isError(), base::EnumToString(base::ErrorCode::VALUE_TO_ERROR));

    return std::get<1>(std::move(_value));
}


template<typename T>
const T& Result<T>::getValue() const&
{
    CHECK(isOk(), base::EnumToString(base::ErrorCode::ERROR_TO_VALUE));

    return std::get<0>(_value);
}


template<typename T>
T&& Result<T>::getValue() &&
{
    CHECK(isOk(), base::EnumToString(base::ErrorCode::ERROR_TO_VALUE));

    return std::get<0>(std::move(_value));
}


} // namespace base
