#include "parameters_helper.hpp"

namespace client
{

ParametersHelper::ParametersHelper(const std::filesystem::path& path, const std::filesystem::path& default_config_path)
{
    if(std::filesystem::exists(path)) {
        _config = base::readConfig(path);
    }
    else if(path != default_config_path) {
        RAISE_ERROR(base::InaccessibleFile, "config was not found by path" + path.string());
    }
}

} // namespace client