#pragma once

#include "utility.hpp"

#include "base/error.hpp"

#include <utility>

namespace base
{

template<typename... Args>
std::size_t Observable<Args...>::subscribe(CallbackType callback)
{
    _observers.push_back({std::move(callback), _next_id});
    return _next_id++;
}


template<typename... Args>
void Observable<Args...>::unsubscribe(std::size_t Id)
{
    if(auto iter = std::find_if(_observers.begin(), _observers.end(),
           [Id](const auto& elem) {
               return elem.second == Id;
           });
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
    for(auto& [callback, id]: _observers) {
        callback(args...);
    }
}

} // namespace base