#pragma once

#include <boost/property_tree/ptree.hpp>

#include <filesystem>
#include <string>

namespace base
{

class PropertyTree
{
    friend void save(const PropertyTree& tree, const std::filesystem::path& path_to_save);

  public:
    using iterator = boost::property_tree::ptree::iterator;
    using const_iterator = boost::property_tree::ptree::const_iterator;

    PropertyTree();

    PropertyTree(const boost::property_tree::ptree& ptree);

    bool hasKey(const std::string& path) const;

    bool empty() const;

    template<typename R>
    R get(const std::string& path) const;

    PropertyTree getSubTree(const std::string& path) const;

    template<typename R>
    void add(const std::string& path, R val);

    void add(const std::string& path, PropertyTree& val);

    void add(const std::string& path, PropertyTree&& val);

    template<typename R>
    std::vector<R> getVector(const std::string& path) const;

    std::string toString() const;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

  private:
    boost::property_tree::ptree _ptree;
};

PropertyTree readConfig(const std::filesystem::path& config_file);

PropertyTree parseJson(const std::string& json_string);

void save(const PropertyTree& tree, const std::filesystem::path& path_to_save);

} // namespace base

#include "property_tree.tpp"
