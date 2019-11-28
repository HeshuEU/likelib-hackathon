#pragma once

#include "parameters_helper.hpp"

namespace rpc_client
{

template<typename Type>
Type ParametersHelper::getValue(const std::string& value_name, const std::string& tag)
{
    if(_config.hasKey(value_name)) {
        auto values = _config.getVector<Type>(value_name);

        if(values.empty()) {
            return getValueFromStdInput<Type>(tag);
        }

        if(values.size() == 1) {
            return values[0];
        }

        std::cout << "choose one of the proposed options[" << tag << "]:" << std::endl;
        for(std::size_t i = 0; i < values.size(); i++) {
            std::cout << i + 1 << " - " << values[i] << std::endl;
        }
        std::cout << values.size() + 1 << " - input other" << std::endl << "chosen option number: ";
        std::string answer;
        std::cin >> answer;
        std::size_t selected_number = 0;
        try {
            selected_number = std::stoi(answer);
        }
        catch(const std::exception& e) {
            RAISE_ERROR(base::InvalidArgument, "invalid option input");
        }

        if((0 < selected_number) && (selected_number < values.size() + 1)) {
            return values[selected_number - 1];
        }
        else if(selected_number == values.size() + 1) {
            return getValueFromStdInput<Type>(tag);
        }
        else {
            RAISE_ERROR(base::InvalidArgument, "invalid option number");
        }
    }
    else {
        return getValueFromStdInput<Type>(tag);
    }
}

template<typename Type>
Type ParametersHelper::getValueFromStdInput(const std::string& tag)
{
    try {
        std::cout << "write [" << tag << "] value: ";
        Type answer;
        std::cin >> answer;
        return answer;
    }
    catch(const std::exception& e) {
        RAISE_ERROR(base::InvalidArgument, "invalid option input");
    }
}


} // namespace rpc_client