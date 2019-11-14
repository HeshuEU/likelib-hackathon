#pragma once

#include "program_options.hpp"

#include "base/error.hpp"

namespace base
{

template<typename ValueType>
ValueType ProgramOptionsParser::getValueByName(const char* flag_name) const
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

} // namespace base