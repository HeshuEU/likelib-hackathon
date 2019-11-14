#include "program_options.hpp"

#include <iostream>

/// Constructor add help option as dafault, and this option will be automatic processed in
/// ProgramOptionsParser::process.
base::ProgramOptionsParser::ProgramOptionsParser() : _options_description("Allowed options")
{
    _options_description.add_options()("help,h", "Print help message");
}

/// Add option that will be interpreted only as std::string and need to get by ProgramOptionsParser::getString call.
/// \param flag name. example: "hash,hs". Such option may be set by: -hs or --hash.
/// \param help message to describe flag meaning. Will be show if set -h or --help options.
void base::ProgramOptionsParser::addStringOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, boost::program_options::value<std::string>(), help);
}

/// Add option that will be interpreted only as int32_t and need to get by ProgramOptionsParser::getInt call.
/// \param flag name. example: "processors,p". Such option may be set by: -p or --processors.
/// \param help message to describe flag meaning. Will be show if set -h or --help options.
void base::ProgramOptionsParser::addIntOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, boost::program_options::value<int32_t>(), help);
}

/// Add option that will be interpreted only as uint32_t and need to get by ProgramOptionsParser::getUint call.
/// \param flag name. example: "processors,p". Such option may be set by: -p or --processors.
/// \param help message to describe flag meaning. Will be show if set -h or --help options.
void base::ProgramOptionsParser::addUintOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, boost::program_options::value<uint32_t>(), help);
}

/// Add option that will be interpreted only as float_t and need to get by ProgramOptionsParser::getFloat call.
/// \param flag name. example: "money,m". Such option may be set by: -m or --money.
/// \param help message to describe flag meaning. Will be show if set -h or --help options.
void base::ProgramOptionsParser::addFloatOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, boost::program_options::value<float_t>(), help);
}

/// Add option that not may be get by any getter method. May will check by ProgramOptionsParser::hasOption call.
/// \param flag name. example: "demonize,d". Such option may be set by: -d or --demonize.
/// \param help message to describe flag meaning. Will be show if set -h or --help options.
void base::ProgramOptionsParser::addFlagOption(const char* flag, const char* help)
{
    _options_description.add_options()(flag, help);
}

/// Process input program options received by application and store to inner vault. Value of options can be got by get
/// methods.
/// \param argc number of input options strings.
/// \param argv array of input options strings world by world.
/// \throw base::ParsingError if options has now valid format or has options/value that was not be set to parser.
/// \throw base::ExpectedClose if input options contains -h or --help option.
void base::ProgramOptionsParser::process(int argc, char** argv)
{
    try {
        auto parsed_options = boost::program_options::parse_command_line(argc, argv, _options_description);
        boost::program_options::store(parsed_options, _options);
    }
    catch(const std::exception& e) {
        RAISE_ERROR(base::ParsingError, e.what());
    }

    boost::program_options::notify(_options);

    if(hasOption("help")) {
        std::cout << _options_description << std::endl;
        RAISE_ERROR(base::ExpectedClose, "Expected close application caused by help option flag");
    }
}

/// Check if contain option
/// \param flag_name option name. Example: if option set by .addUintOption("processors,p", ...) use
/// hasOption("processors") , NOT hasOption("p")
/// \return true if option was be found
bool base::ProgramOptionsParser::hasOption(const std::string& flag_name) const
{
    return hasOption(flag_name.c_str());
}

/// Check if contain option
/// \param flag_name option name. Example: if option set by .addUintOption("processors,p", ...) use
/// hasOption("processors") , NOT hasOption("p")
/// \return true if option was be found
bool base::ProgramOptionsParser::hasOption(const char* flag_name) const
{
    return _options.find(flag_name) != _options.end();
}

/// find value of option that set by addStringOption(flag_name, ...) and return if found.
/// \param flag_name option name. Example: if option set by .addStringOption("hash,h", ...) use getString("hash") , NOT
/// getString("h")
/// \return value of option
/// \throw base::ParsingError if flag_name option was not found
/// \throw base::InvalidArgument if option was not set by addStringOption(flag_name, ...)
std::string base::ProgramOptionsParser::getString(const std::string& flag_name) const
{
    return getString(flag_name.c_str());
}

/// Find value of option that set by addStringOption(flag_name, ...) and return if found.
/// \param flag_name option name. Example: if option set by .addStringOption("hash,h", ...) use getString("hash") , NOT
/// getString("h")
/// \return value of option
/// \throw base::ParsingError if flag_name option was not found
/// \throw base::InvalidArgument if option was not set by addStringOption(flag_name, ...)
std::string base::ProgramOptionsParser::getString(const char* flag_name) const
{
    return getValueByName<std::string>(flag_name);
}

/// Find value of option that set by addIntOption(flag_name, ...) and return if found.
/// \param flag_name option name. Example: if option set by .addIntOption("processors,p", "...) use getInt("processors")
/// , NOT getInt("p")
/// \return value of option
/// \throw base::ParsingError if flag_name option was not found
/// \throw base::InvalidArgument if option was not set by addIntOption(flag_name, ...)
int32_t base::ProgramOptionsParser::getInt(const std::string& flag_name) const
{
    return getInt(flag_name.c_str());
}

/// Find value of option that set by addIntOption(flag_name, ...) and return if found.
/// \param flag_name option name. Example: if option set by .addIntOption("processors,p", "...) use getInt("processors")
/// , NOT getInt("p")
/// \return value of option
/// \throw base::ParsingError if flag_name option was not found
/// \throw base::InvalidArgument if option was not set by addIntOption(flag_name, ...)
int32_t base::ProgramOptionsParser::getInt(const char* flag_name) const
{
    return getValueByName<int32_t>(flag_name);
}

/// Find value of option that set by addUintOption(flag_name, ...) and return if found.
/// \param flag_name option name. Example: if option set by .addUintOption("processors,p", ...) use getInt("processors")
/// , NOT getInt("p")
/// \return value of option
/// \throw base::ParsingError if flag_name option was not found
/// \throw base::InvalidArgument if option was not set by addUintOption(flag_name, ...)
uint32_t base::ProgramOptionsParser::getUint(const std::string& flag_name) const
{
    return getUint(flag_name.c_str());
}

/// Find value of option that set by addUintOption(flag_name, ...) and return if found.
/// \param flag_name option name. Example: if option set by .addUintOption("processors,p", ...) use getInt("processors")
/// , NOT getInt("p")
/// \return value of option
/// \throw base::ParsingError if flag_name option was not found
/// \throw base::InvalidArgument if option was not set by addUintOption(flag_name, ...)
uint32_t base::ProgramOptionsParser::getUint(const char* flag_name) const
{
    return getValueByName<uint32_t>(flag_name);
}

/// Find value of option that set by addFloatOption(flag_name, ...) and return if found.
/// \param flag_name flag_name option name. Example: if option set by .addFloatOption("money,m", ...) use
/// getFloat("money") , NOT getFloat("m")
/// \return value of option
/// \throw base::ParsingError if flag_name option was not found
/// \throw base::InvalidArgument if option was not set by addFloatOption(flag_name, ...)
float_t base::ProgramOptionsParser::getFloat(const std::string& flag_name) const
{
    return getFloat(flag_name.c_str());
}

/// Find value of option that set by addFloatOption(flag_name, ...) and return if found.
/// \param flag_name flag_name option name. Example: if option set by .addFloatOption("money,m", ...) use
/// getFloat("money") , NOT getFloat("m")
/// \return value of option
/// \throw base::ParsingError if flag_name option was not found
/// \throw base::InvalidArgument if option was not set by addFloatOption(flag_name, ...)
float_t base::ProgramOptionsParser::getFloat(const char* flag_name) const
{
    return getValueByName<float_t>(flag_name);
}
