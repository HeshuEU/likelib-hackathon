#include "directory.hpp"

#include "base/error.hpp"
#include "base/log.hpp"

namespace
{
bool checkIfExistingDirectory(const std::filesystem::path& path)
{
    std::error_code ec;
    bool result = std::filesystem::exists(path, ec);
    if(ec) {
        LOG_WARNING << base::Error{base::StatusCode::FUNCTION_CALL_FAILED, ec.message()};
        return false;
    }

    result = result && std::filesystem::is_directory(path, ec);
    if(ec) {
        LOG_WARNING << base::Error{base::StatusCode::FUNCTION_CALL_FAILED, ec.message()};
        return false;
    }

    return result;
}
} // namespace


namespace base
{

Directory::Directory(const std::filesystem::path& path) : std::filesystem::path{path}
{}


Directory::Directory(const std::string_view& path) : std::filesystem::path{path}
{}


void createIfNotExists(const Directory& directory)
{
    std::filesystem::create_directories(directory);
}

} // namespace base