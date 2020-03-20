#include "soft_config.hpp"

#include "base/assert.hpp"

SoftConfig::SoftConfig(const std::filesystem::path& path)
  : base::PropertyTree{ base::readConfig(path) }
{
    // ASSERT(hasKey("root_node"));
}