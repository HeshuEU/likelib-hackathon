#pragma once

#include "base/error.hpp"

#include <boost/program_options.hpp>

namespace base
{

/// \brief Option parser and storage
/// \details Class can set up program options by add... methods, parse and store in inner vault. NOTE: Option help,h
/// reserved by default and auto process by default(see ProgramOptionsParser::process).
class ProgramOptionsParser
{
  public:
    explicit ProgramOptionsParser();

    ~ProgramOptionsParser() = default;

    void addStringOption(const char* flag, const char* help = "");

    void addIntOption(const char* flag, const char* help = "");

    void addUintOption(const char* flag, const char* help = "");

    void addFloatOption(const char* flag, const char* help = "");

    void addFlagOption(const char* flag, const char* help = "");

    void process(int argc, char** argv);

    bool hasOption(const std::string& flag_name) const;

    bool hasOption(const char* flag_name) const;

    std::string getString(const std::string& flag_name) const;

    std::string getString(const char* flag_name) const;

    int32_t getInt(const std::string& flag_name) const;

    int32_t getInt(const char* flag_name) const;

    uint32_t getUint(const std::string& flag_name) const;

    uint32_t getUint(const char* flag_name) const;

    float_t getFloat(const std::string& flag_name) const;

    float_t getFloat(const char* flag_name) const;

  protected:
    boost::program_options::options_description _options_description;
    boost::program_options::variables_map _options;

    template<typename ValueType>
    ValueType getValueByName(const char* flag_name) const
    {
        if(!hasOption(flag_name)) {
            RAISE_ERROR(base::ParsingError, std::string("No option with name: ") + std::string(flag_name));
        }

        try {
            auto option = _options[flag_name].as<ValueType>();
            return option;
        }
        catch(const std::exception& e) {
            RAISE_ERROR(base::InvalidArgument, std::string("Incorrect option type: String"));
        }
    }
};

} // namespace base