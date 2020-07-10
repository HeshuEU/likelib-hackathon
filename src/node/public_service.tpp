#pragma once

#include "public_service.hpp"

namespace tasks{

template<typename Type>
void Queue<Type>::push(std::unique_ptr<Type>&& task)
{
    {
        std::lock_guard lock(_rw_mutex);
        _tasks.emplace_back(std::move(task));
    }
    _has_task.notify_one();
}


template<typename Type>
std::unique_ptr<Type> Queue<Type>::get()
{
    std::lock_guard lock(_rw_mutex);
    std::unique_ptr<Type> current_task{ _tasks.front().release() };
    _tasks.pop_front();
    return current_task;
}


template<typename Type>
void Queue<Type>::wait()
{
    std::unique_lock lock(_rw_mutex);
    _has_task.wait(lock, [this]() { return !_tasks.empty(); });
}


template<typename Type>
bool Queue<Type>::empty() const
{
    std::lock_guard lk(_rw_mutex);
    return _tasks.empty();
}

}