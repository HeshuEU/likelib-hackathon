#include "configuration_manager.hpp"

#include "base/error.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <fstream>

namespace base
{

Configuration::Configuration(const boost::property_tree::ptree& ptree) : _ptree{ptree}
{}

Configuration readConfig(const std::filesystem::path& config_file)
{
    boost::property_tree::ptree ret;
    std::ifstream input(config_file);
    boost::property_tree::read_json(input, ret);
    return ret;
}

bool Configuration::hasKey(const std::string& path) const
{
    try {
        return _ptree.find(std::string{path}) != _ptree.not_found();
    }
    catch(const std::exception& e) {
        RAISE_ERROR("internal error");
    }
}

} // namespace base