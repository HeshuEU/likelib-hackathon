#include "property_tree.hpp"

#include "base/error.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <sstream>

namespace base
{

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
    CLARIFY_ERROR(InaccessibleFile, input.open(config_file), "error opening config file");

    boost::property_tree::ptree ret;
    CLARIFY_ERROR(ParsingError, boost::property_tree::read_json(input, ret), "parsing error");
    return ret;
}


PropertyTree parseJson(const std::string& json_string)
{
    std::istringstream input{ json_string };
    boost::property_tree::ptree ret;
    CLARIFY_ERROR(ParsingError, boost::property_tree::read_json(input, ret), "parsing error");
    return ret;
}


void save(const PropertyTree& tree, const std::filesystem::path& path_to_save)
{
    std::ofstream file;
    CLARIFY_ERROR(InaccessibleFile, file.open(path_to_save), "error opening file to save");
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
    auto& sub = _ptree.get_child(path);
    return PropertyTree(sub);
}


void PropertyTree::add(const std::string& path, PropertyTree& val)
{
    _ptree.put_child(path, val._ptree);
}


void PropertyTree::add(const std::string& path, PropertyTree&& val)
{
    _ptree.put_child(path, val._ptree);
}


std::string PropertyTree::toString() const
{
    std::ostringstream output;
    boost::property_tree::write_json(output, _ptree);
    return output.str();
}


PropertyTree::iterator PropertyTree::begin()
{
    return _ptree.begin();
}


PropertyTree::iterator PropertyTree::end()
{
    return _ptree.end();
}


PropertyTree::const_iterator PropertyTree::begin() const
{
    return _ptree.begin();
}


PropertyTree::const_iterator PropertyTree::end() const
{
    return _ptree.end();
}

} // namespace base