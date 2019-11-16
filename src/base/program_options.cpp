#include "program_options.hpp"

#include <sstream>

namespace base
{
namespace po = boost::program_options;

ProgramOptionsParser::ProgramOptionsParser() : _options_description("Allowed options")
{
    addFlagOption("help", "Print help message");
}

void ProgramOptionsParser::addStringOption(const std::string& flag, const std::string& help)
{
    addOption<std::string>(flag, help);
}

void ProgramOptionsParser::addDefaultStringOption(const std::string& flag, const std::string& default_value,
                                                  const std::string& help)
{
    addDefaultOption<std::string>(flag, help, default_value);
}

void ProgramOptionsParser::addRequiredStringOption(const std::string& flag, const std::string& help)
{
    addRequiredOption<std::string>(flag, help);
}

void ProgramOptionsParser::addIntOption(const std::string& flag, const std::string& help)
{
    addOption<int32_t>(flag, help);
}

void ProgramOptionsParser::addDefaultIntOption(const std::string& flag, int32_t default_value, const std::string& help)
{
    addDefaultOption<int32_t>(flag, help, default_value);
}

void ProgramOptionsParser::addRequiredIntOption(const std::string& flag, const std::string& help)
{
    addRequiredOption<int32_t>(flag, help);
}

void ProgramOptionsParser::addUintOption(const std::string& flag, const std::string& help)
{
    addOption<uint32_t>(flag, help);
}

void ProgramOptionsParser::addDefaultUintOption(const std::string& flag, uint32_t default_value,
                                                const std::string& help)
{
    addDefaultOption<uint32_t>(flag, help, default_value);
}

void ProgramOptionsParser::addRequiredUintOption(const std::string& flag, const std::string& help)
{
    addRequiredOption<uint32_t>(flag, help);
}

void ProgramOptionsParser::addDoubleOption(const std::string& flag, const std::string& help)
{
    addOption<double>(flag, help);
}

void ProgramOptionsParser::addDefaultDoubleOption(const std::string& flag, double default_value,
                                                  const std::string& help)
{
    addDefaultOption<double>(flag, help, default_value);
}

void ProgramOptionsParser::addRequiredDoubleOption(const std::string& flag, const std::string& help)
{
    addRequiredOption<double>(flag, help);
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
        ::boost::program_options::notify(_options);
    }
    catch(const boost::program_options::error& e) {
        RAISE_ERROR(base::ParsingError, e.what());
    }
}

std::string ProgramOptionsParser::getHelpMessage()
{
    std::stringstream ss;
    ss << _options_description << std::endl;
    return ss.str();
}

bool ProgramOptionsParser::hasOption(const std::string& flag_name) const
{
    return _options.find(flag_name) != _options.end();
}

std::string ProgramOptionsParser::getString(const std::string& flag_name) const
{
    return getValueByName<std::string>(flag_name);
}

std::int32_t ProgramOptionsParser::getInt(const std::string& flag_name) const
{
    return getValueByName<std::int32_t>(flag_name);
}

std::uint32_t ProgramOptionsParser::getUint(const std::string& flag_name) const
{
    return getValueByName<std::uint32_t>(flag_name);
}

double ProgramOptionsParser::getDouble(const std::string& flag_name) const
{
    return getValueByName<double>(flag_name);
}

} // namespace base
