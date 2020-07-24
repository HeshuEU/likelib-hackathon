#pragma once

#include <rapidjson/document.h>

#include <filesystem>

class SoftConfig : public rapidjson::Document
{
  public:
    SoftConfig(const std::filesystem::path& path);
};