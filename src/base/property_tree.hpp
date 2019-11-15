#pragma once

#include <boost/property_tree/ptree.hpp>

#include <filesystem>
#include <string>

namespace base
{

class PropertyTree
{
  public:
    PropertyTree(const boost::property_tree::ptree& ptree);

    bool hasKey(const std::string& path) const;

    template<typename R>
    R get(const std::string& path) const;

    template<typename R>
    std::vector<R> getVector(const std::string& path) const;

  private:
    boost::property_tree::ptree _ptree;
};

PropertyTree readConfig(const std::filesystem::path& config_file);
} // namespace base

#include "property_tree.tpp"
