#include "program_options.hpp"

#include <sstream>

namespace base
{

namespace po = boost::program_options;


ProgramOptionsParser::ProgramOptionsParser()
  : _options_description("Allowed options")
{
    addFlag("help", "Print help message");
}


void ProgramOptionsParser::addFlag(const char* flag, const char* help)
{
    _options_description.add_options()(flag, help);
}


void ProgramOptionsParser::process(int argc, const char* const* argv)
{
    try {
        auto parsed_options = ::boost::program_options::parse_command_line(argc, argv, _options_description);
        ::boost::program_options::store(parsed_options, _options);
        ::boost::program_options::notify(_options);
    }
    catch (const boost::program_options::error& e) {
        RAISE_ERROR(base::ParsingError, e.what());
    }
}


std::string ProgramOptionsParser::helpMessage() const
{
    std::stringstream ss;
    ss << _options_description << std::endl;
    return ss.str();
}


bool ProgramOptionsParser::hasOption(const std::string& flag_name) const
{
    return _options.find(flag_name) != _options.end();
}


bool ProgramOptionsParser::empty() const
{
    return _options.empty();
}

} // namespace base
