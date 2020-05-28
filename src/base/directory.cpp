#include "directory.hpp"

namespace base
{

Directory::Directory(const std::filesystem::path& path)
  : std::filesystem::path{ path }
{}


Directory::Directory(const std::string_view& path)
  : std::filesystem::path{ path }
{}


Directory::Directory(const std::string& path)
  : std::filesystem::path{ path }
{}


void createIfNotExists(const Directory& directory)
{
    std::filesystem::create_directories(directory);
}

} // namespace base
