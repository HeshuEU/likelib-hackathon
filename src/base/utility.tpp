#pragma once

#include "utility.hpp"

#include <utility>

namespace base
{

template<typename... Args>
void Observable<Args...>::subscribe(std::function<void(Args...)> callback)
{
    _observers.push_back(std::move(callback));
}


template<typename... Args>
void Observable<Args...>::notify(Args... args)
{
    for(auto& callback: _observers) {
        callback(args...);
    }
}

} // namespace base