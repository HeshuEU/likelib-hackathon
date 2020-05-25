#pragma once

#include "core/address.hpp"
#include "core/types.hpp"

#include "base/bytes.hpp"
#include "base/property_tree.hpp"

#include <evmc/evmc.hpp>

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

namespace vm
{

struct CompiledContract
{
    std::string name;
    base::Bytes code;
    base::PropertyTree metadata;

    explicit CompiledContract() = default;
    explicit CompiledContract(std::string _name);
    explicit CompiledContract(const CompiledContract&) = default;
    explicit CompiledContract(CompiledContract&&) = default;
    ~CompiledContract() = default;
    CompiledContract& operator=(const CompiledContract&) = default;
    CompiledContract& operator=(CompiledContract&&) = default;
};


using Contracts = std::vector<CompiledContract>;


std::optional<Contracts> compile(const std::string& path_to_solidity_file);


evmc::VM load();

} // namespace vm