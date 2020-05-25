#pragma once

#include "base/property_tree.hpp"

#include <filesystem>

class SoftConfig : public base::PropertyTree
{
  public:
    SoftConfig(const std::filesystem::path& path);

  private:
};