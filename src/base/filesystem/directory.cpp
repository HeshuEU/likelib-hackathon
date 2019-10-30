#include "directory.hpp"


namespace
{

    bool checkIfExistingDirectory(const std::filesystem::path& path)
    {
        std::error_code ec;
        std::filesystem::exists(path, ec);

        std::filesystem::is_directory(path, ec);
        return result;
    }

}


namespace base
{

Directory::Directory(const std::filesystem::path& path) : std::filesystem::path{path}
{}


Directory::Directory(const std::string_view& path) : std::filesystem::path{path}
{}

} // namespace base