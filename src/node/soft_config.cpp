#include "soft_config.hpp"

#include "base/error.hpp"

#include <fstream>


SoftConfig::SoftConfig(const std::filesystem::path& path)
  : rapidjson::Document{}
{
    std::ifstream input;
    input.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        input.open(path);
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InaccessibleFile, e.what());
    }

    std::string json_string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    Parse(json_string.c_str());
    if (HasParseError()) {
        RAISE_ERROR(base::InvalidArgument, "json parse error. bad input json string");
    }
}