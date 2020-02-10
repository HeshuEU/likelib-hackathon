#pragma once

#include "base/bytes.hpp"
#include "base/property_tree.hpp"

#include <string>
#include <vector>

namespace vm
{

class Compiler;

class Contract
{
    friend Compiler;

  public:
    Contract() = default;
    Contract(const Contract&) = default;
    Contract(Contract&&) = default;
    Contract& operator=(const Contract&) = default;
    Contract& operator=(Contract&&) = default;

    void setName(const std::string& name);
    const std::string& getName() const;

    void setCode(const base::Bytes& code);
    const base::Bytes& getCode() const;

    void setAbiSpecification(const base::PropertyTree& abi_specification);
    const base::PropertyTree& getAbiSpecification() const;

    void setMetadata(const base::PropertyTree& metadata);
    const base::PropertyTree& getMetadata() const;

    void setSignatures(const std::vector<std::pair<std::string, std::string>>& signatures);
    const std::vector<std::pair<std::string, std::string>>& getSignatures() const;

  private:
    std::string _name;
    base::Bytes _bytecode;
    base::PropertyTree _abi_specification;
    base::PropertyTree _metadata;
    std::vector<std::pair<std::string, std::string>> _signatures;
};

using Contracts = std::vector<Contract>;

class Compiler
{
  public:
    Compiler(const std::string& path_to_solc);

    Compiler(const Compiler&) = default;
    Compiler(Compiler&&) = default;
    Compiler& operator=(const Compiler&) = default;
    Compiler& operator=(Compiler&&) = default;

    std::optional<Contracts> compile(const std::string& path_to_solidity_code) const;

  private:
    std::string _path_to_solc;

    std::vector<std::string> call_command(std::vector<std::string> args) const;

    std::vector<std::pair<std::string, base::Bytes>> call_compilation_command(
        const std::string& path_to_solidity_code) const;

    std::vector<std::pair<std::string, base::PropertyTree>> call_metadata_command(
        const std::string& path_to_solidity_code) const;

    std::vector<std::pair<std::string, base::PropertyTree>> call_abi_command(
        const std::string& path_to_solidity_code) const;

    std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> call_hashes_command(
        const std::string& path_to_solidity_code) const;
};

} // namespace vm