#include "parameters_helper.hpp"

namespace rpc_client
{

ParametersHelper::ParametersHelper(const std::filesystem::path& path)
{
    if(std::filesystem::exists(path)) {
        _config = base::readConfig(path);
    }
    else {
        std::cerr << "Warning. Config was not found." << std::endl;
    }
}

} // namespace rpc_client