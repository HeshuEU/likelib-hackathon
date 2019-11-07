#include "property_tree.hpp"

#include "base/error.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <fstream>

namespace base
{

PropertyTree::PropertyTree(const boost::property_tree::ptree& ptree) : _ptree{ptree}
{}

PropertyTree readConfig(const std::filesystem::path& config_file)
{
    std::ifstream input;
    input.exceptions(std::ios::badbit | std::ios::failbit);
    CLARIFY_ERROR(input.open(config_file), "error opening config file");

    boost::property_tree::ptree ret;
    CLARIFY_ERROR(boost::property_tree::read_json(input, ret), "parsing error");
    return ret;
}

bool PropertyTree::hasKey(const std::string& path) const
{
    CLARIFY_ERROR(return _ptree.find(std::string{path}) != _ptree.not_found(), "internal error");
}

} // namespace base