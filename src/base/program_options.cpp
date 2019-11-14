#include "program_options.hpp"

#include <iostream>

namespace base
{

ProgramOptionsParser::ProgramOptionsParser() : _options_description("Allowed options")
{
    _options_description.add_options()("help,h", "Print help message");
}

void ProgramOptionsParser::addStringOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, boost::program_options::value<std::string>(), help);
}

void ProgramOptionsParser::addIntOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, boost::program_options::value<int32_t>(), help);
}

void ProgramOptionsParser::addUintOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, boost::program_options::value<uint32_t>(), help);
}

void ProgramOptionsParser::addFloatOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, boost::program_options::value<float_t>(), help);
}

void ProgramOptionsParser::addFlagOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, help);
}

void ProgramOptionsParser::process(int argc, char** argv)
{
    try {
        auto parsed_options = ::boost::program_options::parse_command_line(argc, argv, _options_description);
        ::boost::program_options::store(parsed_options, _options);
    }
    catch(const std::exception& e) {
        RAISE_ERROR(base::ParsingError, e.what());
    }

    ::boost::program_options::notify(_options);

    if(hasOption("help")) {
        std::cout << _options_description << std::endl;
        RAISE_ERROR(base::ExpectedClose, "Expected close application caused by help option flag");
    }
}

bool ProgramOptionsParser::hasOption(const std::string& flag_name) const
{
    return hasOption(flag_name.c_str());
}

bool ProgramOptionsParser::hasOption(const char* flag_name) const
{
    return _options.find(flag_name) != _options.end();
}

std::string ProgramOptionsParser::getString(const std::string& flag_name) const
{
    return getString(flag_name.c_str());
}

std::string ProgramOptionsParser::getString(const char* flag_name) const
{
    return getValueByName<std::string>(flag_name);
}

int32_t ProgramOptionsParser::getInt(const std::string& flag_name) const
{
    return getInt(flag_name.c_str());
}

int32_t ProgramOptionsParser::getInt(const char* flag_name) const
{
    return getValueByName<int32_t>(flag_name);
}

uint32_t ProgramOptionsParser::getUint(const std::string& flag_name) const
{
    return getUint(flag_name.c_str());
}

uint32_t ProgramOptionsParser::getUint(const char* flag_name) const
{
    return getValueByName<uint32_t>(flag_name);
}

float_t ProgramOptionsParser::getFloat(const std::string& flag_name) const
{
    return getFloat(flag_name.c_str());
}

float_t ProgramOptionsParser::getFloat(const char* flag_name) const
{
    return getValueByName<float_t>(flag_name);
}

} // namespace base