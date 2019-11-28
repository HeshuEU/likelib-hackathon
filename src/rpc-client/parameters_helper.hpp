#pragma once

#include "base/property_tree.hpp"

#include <filesystem>
#include <iostream>

namespace rpc_client
{

class ParametersHelper
{
  public:
    ParametersHelper();

    ParametersHelper(const std::filesystem::path& path);

    template<typename Type>
    Type getValue(const std::string& value_name, const std::string& tag);

    template<typename Type>
    Type getValueFromStdInput(const std::string& tag);
  private:
    base::PropertyTree _config;
};

} // namespace rpc_client

#include "parameters_helper.tpp"