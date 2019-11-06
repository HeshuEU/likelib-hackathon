#pragma once

#include <boost/property_tree/ptree.hpp>

#include <filesystem>
#include <string>

namespace base
{

class Configuration
{
  public:
    Configuration(const boost::property_tree::ptree& ptree);

    bool hasKey(const std::string& path) const;

    template<typename R>
    R get(const std::string& path);

  private:
    boost::property_tree::ptree _ptree;
};

Configuration readConfig(const std::filesystem::path& config_file);
} // namespace base

#include "configuration_manager.tpp"
