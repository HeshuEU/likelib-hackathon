#include "program_options.hpp"

#include <iostream>

namespace base
{

ProgramOptionsParser::ProgramOptionsParser() : _options_description("Allowed options")
{
    _options_description.add_options()("help,h", "Print help message");
}

void ProgramOptionsParser::addStringOption(const std::string& flag, const std::string& help)
{
    _options_description.add_options()(flag.c_str(), boost::program_options::value<std::string>(), help.c_str());
}

void ProgramOptionsParser::addIntOption(const std::string& flag, const std::string& help)
{
    _options_description.add_options()(flag.c_str(), boost::program_options::value<int32_t>(), help.c_str());
}

void ProgramOptionsParser::addUintOption(const std::string& flag, const std::string& help)
{
    _options_description.add_options()(flag.c_str(), boost::program_options::value<uint32_t>(), help.c_str());
}

void ProgramOptionsParser::addFloatOption(const std::string& flag, const std::string& help)
{
    _options_description.add_options()(flag.c_str(), boost::program_options::value<float_t>(), help.c_str());
}

void ProgramOptionsParser::addFlagOption(const std::string& flag, const std::string& help)
{
    _options_description.add_options()(flag.c_str(), help.c_str());
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
    return _options.find(flag_name) != _options.end();
}

std::string ProgramOptionsParser::getString(const std::string& flag_name) const
{
    return getValueByName<std::string>(flag_name);
}

int32_t ProgramOptionsParser::getInt(const std::string& flag_name) const
{
    return getValueByName<int32_t>(flag_name);
}

uint32_t ProgramOptionsParser::getUint(const std::string& flag_name) const
{
    return getValueByName<uint32_t>(flag_name);
}

float_t ProgramOptionsParser::getFloat(const std::string& flag_name) const
{
    return getValueByName<float_t>(flag_name);
}

} // namespace base