#include "parameters_helper.hpp"

namespace rpc_client
{

ParametersHelper::ParametersHelper()
{}


ParametersHelper::ParametersHelper(const std::filesystem::path& path) : _config(base::readConfig(path))
{}

}