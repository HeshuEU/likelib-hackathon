#pragma once

#include "utility.hpp"

#include "base/error.hpp"

#include <utility>

namespace base
{

template<typename... Args>
std::size_t Observable<Args...>::subscribe(CallbackType callback)
{
    _observers.push_back({ std::move(callback), _next_id });
    return _next_id++;
}


template<typename... Args>
void Observable<Args...>::unsubscribe(std::size_t Id)
{
    if (auto iter =
          std::find_if(_observers.begin(), _observers.end(), [Id](const auto& elem) { return elem.second == Id; });
        iter != _observers.end()) {
        _observers.erase(iter);
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "There is no Callback with an Id");
    }
}


template<typename... Args>
void Observable<Args...>::notify(Args... args)
{
    for (auto& [callback, id] : _observers) {
        callback(args...);
    }
}


template<typename T>
bool OwningPoolMt<T>::add(std::shared_ptr<T> value)
{
    std::unique_lock lk(_pool_mutex);
    auto ptr = value.get();
    return _pool.insert({ ptr, std::move(value) }).second;
}


template<typename T>
void OwningPoolMt<T>::remove(const T* value)
{
    std::unique_lock lk(_pool_mutex);
    if (auto it = _pool.find(value); it == _pool.end()) {
        RAISE_ERROR(base::ValueNotFound, "no such value in pool");
    }
    else {
        _pool.erase(it);
    }
}


template<typename T>
bool OwningPoolMt<T>::tryRemove(const T* value)
{
    std::unique_lock lk(_pool_mutex);
    if (auto it = _pool.find(value); it == _pool.end()) {
        return false;
    }
    else {
        _pool.erase(it);
        return false;
    }
}


template<typename T>
bool OwningPoolMt<T>::contains(const T* value) const
{
    std::shared_lock lk(_pool_mutex);
    return _pool.find(value) != _pool.end();
}


} // namespace base