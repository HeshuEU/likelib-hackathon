#include "utility.hpp"

std::optinal<std::filesystem::path> checkAndGetFilePath(const base::SubprogramRouter& router, const std::string_view& option_name)
{
    if(router.getOptionsParser()->hasOption(option_name)) {

    }
    else {
        return std::nullopt;
    }
}


std::filesystem::path checkAndGetDirectoryPath(const base::SubprogramRouter& router, const std::string_view& option_name)
{

}
