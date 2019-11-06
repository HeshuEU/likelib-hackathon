#pragma once

#include "configuration_manager.hpp"

#include "base/error.hpp"

namespace base
{

template<typename R>
R Configuration::get(const std::string& path)
{
    try {
        return _ptree.get<R>(std::string{path});
    }
    catch(const std::exception& e) {
        RAISE_ERROR(std::string{"Cannot get given value from configuration. Additional info = "} + e.what());
    }
}

} // namespace base