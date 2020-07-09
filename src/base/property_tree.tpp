#pragma once

#include "property_tree.hpp"

#include "base/error.hpp"

namespace base
{

template<typename R>
R PropertyTree::get(const std::string& path) const
{
    try {
        return _ptree.get<R>(std::string{ path });
    }
    catch (const std::exception& e) {
        RAISE_ERROR(KeyNotFound, e.what(), path);
    }
}


template<typename R>
void PropertyTree::add(const std::string& path, R val)
{
    _ptree.put(path, val);
}


template<typename R>
std::vector<R> PropertyTree::getVector(const std::string& path) const
{
    try {
        std::vector<R> ret;
        for (const auto& item : _ptree.get_child(path)) {
            ret.push_back(item.second.get_value<R>());
        }
        return ret;
    }
    catch (const std::exception& e) {
        RAISE_ERROR(KeyNotFound, e.what(), path);
    }
}


} // namespace base
