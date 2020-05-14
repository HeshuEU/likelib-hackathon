#include "config.hpp"


namespace
{
constexpr const char* const KEY_FILE_PREFIX = "lkkey";
}


namespace base::config
{

//std::filesystem::path makePublicKeyPath(const std::filesystem::path& directory_path)
//{
//    auto ret = directory_path / KEY_FILE_PREFIX;
//    ret.replace_extension(".pub");
//    return ret;
//}
//

std::filesystem::path makePrivateKeyPath(const std::filesystem::path& directory_path)
{
    return directory_path / KEY_FILE_PREFIX;
}

} // namespace base::config
