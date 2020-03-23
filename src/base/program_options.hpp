#pragma once

#include <boost/program_options.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace base
{

/// \brief Option parser and storage
/// \details Class can set up program options by add... methods, parse and store in inner vault. NOTE: Option --help
/// reserved by default.
class ProgramOptionsParser
{
  public:
    /// As default constructor.
    explicit ProgramOptionsParser();

    ~ProgramOptionsParser() = default;

    /// Add optional option that will as flag check by hasOption
    /// \param flag name. example: "useGpu,g". Such option may be set by: -g or --useGpu.
    /// \param help message to describe flag meaning. Will be show if set --help options.
    void addFlag(const char* flag, const char* help);

    /// Add optional option that will be interpreted only as std::string and need to get by
    /// ProgramOptionsParser::getString call. \param flag name. example: "hash,h". Such option may be set by: -h or
    /// --hash. \param help message to describe flag meaning. Will be show if set --help options.
    template<typename ValueType>
    void addOption(const std::string& flag, const std::string& help);

    /// Add option that has default value in case if it was not set in input data.
    /// Other such as ProgramOptionsParser::addStringOption
    /// \param flag name. example: "hash,h". Such option may be set by: -h or --hash.
    /// \param help message to describe flag meaning. Will be show if set --help options.
    template<typename ValueType>
    void addOption(const std::string& flag, ValueType defaultValue, const std::string& help);

    /// Add required option that will be interpreted only as float_t and need to get by ProgramOptionsParser::getFloat
    /// call.
    /// \param flag name. example: "money,m". Such option may be set by: -m or --money.
    /// \param help message to describe flag meaning. Will be show if set --help options.
    template<typename ValueType>
    void addRequiredOption(const std::string& flag, const std::string& help);

    /// Process input program options received by application and store to inner vault. Value of options can be got by
    /// get methods.
    /// \param argc number of input options strings.
    /// \param argv array of input options strings world by world.
    /// \return exit code of sub process defined by createSubParser or ok.
    /// \throw base::ParsingError if options has now valid format or has options/value that was not be set to parser.
    /// \throw base::InvalidArgument if argument is not an option and sub command not found
    void process(int argc, const char* const* argv);

    /// generate help message for options defined previously
    /// \return help message
    std::string helpMessage() const;

    /// Check if contain option
    /// \param flag_name option name. Example: if option set by .addUintOption("processors,p", ...) use
    /// hasOption("processors") , NOT hasOption("p")
    /// \return true if option was be found
    bool hasOption(const std::string& flag_name) const;

    /// Check if no any options was not input
    /// \return false if no one options was not input
    bool empty() const;

    /// Find value of option that set by addFloatOption(flag_name, ...) and return if found.
    /// \param flag_name flag_name option name. Example: if option set by .addFloatOption("money,m", ...) use
    /// getFloat("money") , NOT getFloat("m")
    /// \return value of option
    /// \throw base::ParsingError if flag_name option was not found
    /// \throw base::InvalidArgument if option was not set by addFloatOption(flag_name, ...)
    template<typename ValueType>
    ValueType getValue(const std::string& flag_name) const;

  private:
    boost::program_options::options_description _options_description;
    boost::program_options::variables_map _options;
};

} // namespace base

#include "program_options.tpp"
