#include "directory.hpp"

#include "base/error.hpp"
#include "base/log.hpp"

namespace
{
bool checkIfExistingDirectory(const std::filesystem::path& path)
{
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
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