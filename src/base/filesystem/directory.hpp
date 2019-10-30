#pragma once

#include <filesystem>
#include <string_view>

namespace base
{

class Directory : public std::filesystem::path
{
  public:
    Directory(const std::filesystem::path& path);
    Directory(const std::string_view& path);
};


} // namespace base