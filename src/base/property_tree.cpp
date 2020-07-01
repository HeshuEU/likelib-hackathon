#include "property_tree.hpp"

#include "base/error.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <sstream>

namespace base
{

PropertyTree::KeyNotFound::KeyNotFound(const char* file_name,
                                       std::size_t line_number,
                                       const char* function_signature,
                                       std::string message,
                                       std::string path)
  : base::Error(file_name, line_number, function_signature, std::move(message))
  , _path{ std::move(path) }
{}


PropertyTree::PropertyTree()
  : _ptree{}
{}


PropertyTree::PropertyTree(const boost::property_tree::ptree& ptree)
  : _ptree{ ptree }
{}


PropertyTree readConfig(const std::filesystem::path& config_file)
{
    std::ifstream input;
    input.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        input.open(config_file);
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InaccessibleFile, e.what());
    }

    boost::property_tree::ptree ret;
    try {
        boost::property_tree::read_json(input, ret);
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::ParsingError, e.what());
    }
    return ret;
}


PropertyTree parseJson(const std::string& json_string)
{
    std::istringstream input{ json_string };
    boost::property_tree::ptree ret;
    try {
        boost::property_tree::read_json(input, ret);
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::ParsingError, e.what());
    }
    return ret;
}


void save(const PropertyTree& tree, const std::filesystem::path& path_to_save)
{
    std::ofstream file;
    file.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        file.open(path_to_save);
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InaccessibleFile, e.what());
    }
    boost::property_tree::write_json(file, tree._ptree);
    file.close();
}


bool PropertyTree::hasKey(const std::string& path) const
{
    return static_cast<bool>(_ptree.get_child_optional(path));
}


bool PropertyTree::empty() const
{
    return _ptree.empty();
}


PropertyTree PropertyTree::getSubTree(const std::string& path) const
{
    return PropertyTree(_ptree.get_child(path));
}


std::string PropertyTree::toString() const
{
    std::ostringstream output;
    boost::property_tree::write_json(output, _ptree);
    return output.str();
}

} // namespace base