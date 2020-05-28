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
bool OwningPool<T>::own(std::shared_ptr<T> value)
{
    std::unique_lock lk(_pool_mutex);
    auto ptr = value.get();
    return _pool.insert({ ptr, std::move(value) }).second;
}


template<typename T>
void OwningPool<T>::disown(const T* value)
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
bool OwningPool<T>::tryDisown(const T* value)
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
bool OwningPool<T>::isOwning(const T* value) const
{
    std::shared_lock lk(_pool_mutex);
    return _pool.find(value) != _pool.end();
}


template<typename T>
void OwningPool<T>::forEach(std::function<void(const T&)> f) const
{
    std::shared_lock lk(_pool_mutex);
    for (const auto& p : _pool) {
        f(*p.second);
    }
}


template<typename T>
void OwningPool<T>::forEach(std::function<void(T&)> f)
{
    std::unique_lock lk(_pool_mutex);
    for (const auto& p : _pool) {
        f(*p.second);
    }
}


template<typename T>
void OwningPool<T>::disownIf(std::function<bool(const T&)> f) const
{
    std::unique_lock lk(_pool_mutex);
    for (auto it = _pool.begin(); it != _pool.end();) {
        if (f(*it->second)) {
            it = _pool.erase(it);
        }
        else {
            ++it;
        }
    }
}


template<typename T>
void OwningPool<T>::disownIf(std::function<bool(T&)> f)
{
    std::unique_lock lk(_pool_mutex);
    for (auto it = _pool.begin(); it != _pool.end();) {
        if (f(*it->second)) {
            it = _pool.erase(it);
        }
        else {
            ++it;
        }
    }
}


template<typename T>
void OwningPool<T>::clear()
{
    std::unique_lock lk(_pool_mutex);
    _pool.clear();
}


} // namespace base