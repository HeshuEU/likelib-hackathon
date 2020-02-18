#pragma once

#include "base/property_tree.hpp"

#include <filesystem>

class ParametersHelper
{
  public:
    //=====================
    ParametersHelper(const std::filesystem::path& path, const std::filesystem::path& default_config_path);
    //========================
    template<typename Type>
    Type getValue(const std::string& value_name, const std::string& tag);

    //========================
  private:
    //=========================
    base::PropertyTree _config;
    //=======================
    template<typename Type>
    Type getValueFromStdInput(const std::string& tag);
    //=======================
};

#include "parameters_helper.tpp"