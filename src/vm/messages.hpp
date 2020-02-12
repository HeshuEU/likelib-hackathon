#pragma once

#include "base/bytes.hpp"
#include "base/property_tree.hpp"

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

namespace vm
{

class Solc;

class CompiledContract
{
  public:
    CompiledContract() = default;
    CompiledContract(const CompiledContract&) = default;
    CompiledContract(CompiledContract&&) = default;
    CompiledContract& operator=(const CompiledContract&) = default;
    CompiledContract& operator=(CompiledContract&&) = default;

    void setName(const std::string& name);
    const std::string& getName() const;

    void setRuntimeCode(const base::Bytes& code);
    const base::Bytes& getRuntimeCode() const;

    void setFullCode(const base::Bytes& code);
    const base::Bytes& getFullCode() const;

    void setAbiSpecification(const base::PropertyTree& abi_specification);
    const base::PropertyTree& getAbiSpecification() const;

    void setMetadata(const base::PropertyTree& metadata);
    const base::PropertyTree& getMetadata() const;

    void setSignatures(const std::vector<std::pair<base::Bytes, std::string>>& signatures);
    const std::vector<std::pair<base::Bytes, std::string>>& getSignatures() const;

    //TODO messages builder

  private:
    std::string _name;
    base::Bytes _runtime_code;
    base::Bytes _full_code;
    base::PropertyTree _abi_specification;
    base::PropertyTree _metadata;
    std::vector<std::pair<base::Bytes, std::string>> _signatures;
};

using Contracts = std::vector<CompiledContract>;

class Solc
{
  public:
    explicit Solc();

    Solc(const Solc&) = default;
    Solc(Solc&&) = default;
    Solc& operator=(const Solc&) = default;
    Solc& operator=(Solc&&) = default;

    std::optional<Contracts> compile(const std::string& path_to_solidity_code) const;

  private:
    static constexpr const char* _SOLC_NAME = "solc";
    const boost::filesystem::path _path_to_solc;

    std::vector<std::string> call_command(std::vector<std::string> args) const;

    std::vector<std::pair<std::string, base::Bytes>> call_runtime_compilation_command(
        const std::string& path_to_solidity_code) const;

    std::vector<std::pair<std::string, base::Bytes>> call_full_compilation_command(
        const std::string& path_to_solidity_code) const;

    std::vector<std::pair<std::string, base::PropertyTree>> call_metadata_command(
        const std::string& path_to_solidity_code) const;

    std::vector<std::pair<std::string, base::PropertyTree>> call_abi_command(
        const std::string& path_to_solidity_code) const;

    std::vector<std::pair<std::string, std::vector<std::pair<base::Bytes, std::string>>>> call_hashes_command(
        const std::string& path_to_solidity_code) const;
};

} // namespace vm