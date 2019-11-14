#pragma once

#include <boost/program_options.hpp>

#include <cstdint>

namespace base
{

/// \brief Option parser and storage
/// \details Class can set up program options by add... methods, parse and store in inner vault. NOTE: Option help,h
/// reserved by default and auto process by default(see ProgramOptionsParser::process).
class ProgramOptionsParser
{
  public:
    /// Constructor add help option as default, and this option will be automatic processed in
    /// ProgramOptionsParser::process.
    explicit ProgramOptionsParser();

    ~ProgramOptionsParser() = default;

    /// Add option that will be interpreted only as std::string and need to get by ProgramOptionsParser::getString call.
    /// \param flag name. example: "hash,hs". Such option may be set by: -hs or --hash.
    /// \param help message to describe flag meaning. Will be show if set -h or --help options.
    void addStringOption(const std::string& flag, const std::string& help = "");

    /// Add option that will be interpreted only as int32_t and need to get by ProgramOptionsParser::getInt call.
    /// \param flag name. example: "processors,p". Such option may be set by: -p or --processors.
    /// \param help message to describe flag meaning. Will be show if set -h or --help options.
    void addIntOption(const std::string& flag, const std::string& help = "");

    /// Add option that will be interpreted only as uint32_t and need to get by ProgramOptionsParser::getUint call.
    /// \param flag name. example: "processors,p". Such option may be set by: -p or --processors.
    /// \param help message to describe flag meaning. Will be show if set -h or --help options.
    void addUintOption(const std::string& flag, const std::string& help = "");

    /// Add option that will be interpreted only as float_t and need to get by ProgramOptionsParser::getFloat call.
    /// \param flag name. example: "money,m". Such option may be set by: -m or --money.
    /// \param help message to describe flag meaning. Will be show if set -h or --help options.
    void addDoubleOption(const std::string& flag, const std::string& help = "");

    /// Add option that not may be get by any getter method. May will check by ProgramOptionsParser::hasOption call.
    /// \param flag name. example: "demonize,d". Such option may be set by: -d or --demonize.
    /// \param help message to describe flag meaning. Will be show if set -h or --help options.
    void addFlagOption(const std::string& flag, const std::string& help = "");

    /// Process input program options received by application and store to inner vault. Value of options can be got by
    /// get methods.
    /// \param argc number of input options strings.
    /// \param argv array of input options strings world by world.
    /// \throw base::ParsingError if options has now valid format or has options/value that was not be set to parser.
    /// \throw base::ExpectedClose if input options contains -h or --help option.
    void process(int argc, char** argv);

    /// Check if contain option
    /// \param flag_name option name. Example: if option set by .addUintOption("processors,p", ...) use
    /// hasOption("processors") , NOT hasOption("p")
    /// \return true if option was be found
    bool hasOption(const std::string& flag_name) const;

    /// find value of option that set by addStringOption(flag_name, ...) and return if found.
    /// \param flag_name option name. Example: if option set by .addStringOption("hash,h", ...) use getString("hash") ,
    /// NOT getString("h")
    /// \return value of option
    /// \throw base::ParsingError if flag_name option was not found
    /// \throw base::InvalidArgument if option was not set by addStringOption(flag_name, ...)
    std::string getString(const std::string& flag_name) const;

    /// Find value of option that set by addIntOption(flag_name, ...) and return if found.
    /// \param flag_name option name. Example: if option set by .addIntOption("processors,p", "...) use
    /// getInt("processors"), NOT getInt("p")
    /// \return value of option
    /// \throw base::ParsingError if flag_name option was not found
    /// \throw base::InvalidArgument if option was not set by addIntOption(flag_name, ...)
    std::int32_t getInt(const std::string& flag_name) const;

    /// Find value of option that set by addUintOption(flag_name, ...) and return if found.
    /// \param flag_name option name. Example: if option set by .addUintOption("processors,p", ...) use
    /// getInt("processors") , NOT getInt("p")
    /// \return value of option
    /// \throw base::ParsingError if flag_name option was not found
    /// \throw base::InvalidArgument if option was not set by addUintOption(flag_name, ...)
    std::uint32_t getUint(const std::string& flag_name) const;

    /// Find value of option that set by addFloatOption(flag_name, ...) and return if found.
    /// \param flag_name flag_name option name. Example: if option set by .addFloatOption("money,m", ...) use
    /// getFloat("money") , NOT getFloat("m")
    /// \return value of option
    /// \throw base::ParsingError if flag_name option was not found
    /// \throw base::InvalidArgument if option was not set by addFloatOption(flag_name, ...)
    double getDouble(const std::string& flag_name) const;

  private:
    boost::program_options::options_description _options_description;
    boost::program_options::variables_map _options;

    template<typename ValueType>
    ValueType getValueByName(const std::string& flag_name) const;
};

} // namespace base

#include "program_options.tpp"
