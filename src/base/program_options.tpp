#pragma once

#include "program_options.hpp"

#include "base/error.hpp"

namespace base
{

template<typename ValueType>
void ProgramOptionsParser::addOption(const std::string& flag, const std::string& help)
{
    _options_description.add_options()(flag.c_str(), boost::program_options::value<ValueType>(), help.c_str());
}


template<typename ValueType>
void ProgramOptionsParser::addOption(const std::string& flag, ValueType defaultValue, const std::string& help)
{
    _options_description.add_options()(
      flag.c_str(), boost::program_options::value<ValueType>()->default_value(defaultValue), help.c_str());
}


template<typename ValueType>
void ProgramOptionsParser::addRequiredOption(const std::string& flag, const std::string& help)
{
    _options_description.add_options()(
      flag.c_str(), boost::program_options::value<ValueType>()->required(), help.c_str());
}


template<typename ValueType>
ValueType ProgramOptionsParser::getValue(const std::string& flag_name) const
{
    if (!hasOption(flag_name)) {
        RAISE_ERROR(base::ParsingError, std::string("No option with name: ") + flag_name);
    }

    try {
        auto option = _options[flag_name].as<ValueType>();
        return option;
    }
    catch (const boost::program_options::error& e) {
        RAISE_ERROR(base::InvalidArgument, std::string("Incorrect option type: String"));
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InvalidArgument, e.what());
    }
    catch (...) {
        RAISE_ERROR(base::InvalidArgument, "[unexpected error]");
    }
}

} // namespace base
