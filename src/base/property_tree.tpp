#pragma once

#include "property_tree.hpp"

#include "base/error.hpp"

namespace base
{

template<typename R>
R PropertyTree::get(const std::string& path)
{
    try {
        return _ptree.get<R>(std::string{path});
    }
    catch(const std::exception& e) {
        RAISE_ERROR(InvalidArgument,
                    std::string{"Cannot get given value from configuration. Additional info = "} + e.what());
    }
}

} // namespace base