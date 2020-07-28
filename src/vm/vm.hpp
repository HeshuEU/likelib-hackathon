#pragma once

#include "core/address.hpp"
#include "core/types.hpp"

#include "base/bytes.hpp"

#include <evmc/evmc.hpp>

#include <rapidjson/document.h>

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

namespace vm
{

struct CompiledContract
{
    std::string name;
    base::Bytes code;
    rapidjson::Document metadata;

    explicit CompiledContract() = default;
    explicit CompiledContract(const std::string& _name);
    explicit CompiledContract(const CompiledContract&) = default;
    explicit CompiledContract(CompiledContract&&) = default;
    ~CompiledContract() = default;
    CompiledContract& operator=(const CompiledContract&) = delete;
    CompiledContract& operator=(CompiledContract&&) = default;
};


using Contracts = std::vector<CompiledContract>;


std::optional<Contracts> compile(const std::string& path_to_solidity_file);


evmc::VM load();

} // namespace vm