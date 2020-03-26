#include "messages.hpp"

#include <base/assert.hpp>

#include <boost/process.hpp>

#include <set>

namespace bp = ::boost::process;


namespace
{
std::tuple<std::string, std::string> parseInfoLine(const std::string& path)
{
    constexpr std::size_t DELIM_SIZE = 8;
    auto shrinked = path.substr(DELIM_SIZE, path.size() - DELIM_SIZE * 2);
    auto delimiter_pos = shrinked.find(':');
    auto source_file = shrinked.substr(0, delimiter_pos);
    auto contract_name = shrinked.substr(delimiter_pos + 1, shrinked.size());
    return std::make_tuple(source_file, contract_name);
}

std::pair<base::Bytes, std::string> parseContractMembers(const std::string& contract_members)
{

    if(auto space_pos = contract_members.find(' '); space_pos != contract_members.npos) {
        auto hash = base::fromHex<base::Bytes>(contract_members.substr(0, space_pos - 1));
        auto signature = contract_members.substr(space_pos + 1, contract_members.size() - space_pos - 1);
        return {hash, signature};
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "Invalid string for contract members parsing");
    }
}
} // namespace

namespace vm
{

void CompiledContract::setName(const std::string& name)
{
    _name = name;
}


const std::string& CompiledContract::getName() const
{
    return _name;
}


void CompiledContract::setRuntimeCode(const base::Bytes& code)
{
    _runtime_code = code;
}


const base::Bytes& CompiledContract::getRuntimeCode() const
{
    return _runtime_code;
}


void CompiledContract::setFullCode(const base::Bytes& code)
{
    _full_code = code;
}


const base::Bytes& CompiledContract::getFullCode() const
{
    return _full_code;
}


void CompiledContract::setAbiSpecification(const base::PropertyTree& abi_specification)
{
    _abi_specification = abi_specification;
}


const base::PropertyTree& CompiledContract::getAbiSpecification() const
{
    return _abi_specification;
}


void CompiledContract::setMetadata(const base::PropertyTree& metadata)
{
    _metadata = metadata;
}


const base::PropertyTree& CompiledContract::getMetadata() const
{
    return _metadata;
}


void CompiledContract::setSignatures(const std::vector<std::pair<base::Bytes, std::string>>& signatures)
{
    _signatures = signatures;
}


const std::vector<std::pair<base::Bytes, std::string>>& CompiledContract::getSignatures() const
{
    return _signatures;
}


Solc::Solc()
  : _path_to_solc{ bp::search_path(_SOLC_NAME) }
{
    if (!boost::filesystem::exists(_path_to_solc)) {
        RAISE_ERROR(base::InaccessibleFile, "Solidity compiler was not found");
    }
}


std::optional<Contracts> Solc::compile(const std::string& path_to_solidity_code) const
{
    auto full_compilation_output = call_full_compilation_command(path_to_solidity_code);
    auto metadata_output = call_metadata_command(path_to_solidity_code);

    Contracts contracts{};
    for (const auto& contract_name_full_code_pair : full_compilation_output) {
        CompiledContract contract;
        contract.setName(contract_name_full_code_pair.first);
        contract.setFullCode(contract_name_full_code_pair.second);

        for(const auto& contract_name_metadata_pair: metadata_output) {
            if(contract_name_metadata_pair.first == contract.getName()) {
                contract.setMetadata(contract_name_metadata_pair.second);
            }
        }

        contracts.push_back(std::move(contract));
    }

    return contracts;
}


std::vector<std::string> Solc::call_command(std::vector<std::string> args) const
{
    bp::ipstream out;
    bp::child c(_path_to_solc, args, bp::std_out > out);
    c.wait();

    std::vector<std::string> out_put_result_values;
    std::string out_result;
    while(std::getline(out, out_result)) {
        out_put_result_values.push_back(out_result);
    }

    if (c.exit_code()) {
        std::ostringstream s;
        for (const auto& item : out_put_result_values) {
            s << item;
        }
        RAISE_ERROR(base::SystemCallFailed, s.str());
    }

    return out_put_result_values;
}


std::vector<std::pair<std::string, base::Bytes>> Solc::call_full_compilation_command(
  const std::string& path_to_solidity_code) const
{
    std::vector<std::string> args{"--bin", path_to_solidity_code};
    auto res = call_command(args);

    std::vector<std::pair<std::string, base::Bytes>> contracts_byte_codes;
    constexpr const size_t GROUP_SIZE = 4;
    for(std::size_t i = 0; i < res.size() / GROUP_SIZE; i++) {
        auto current_contract_index = i * GROUP_SIZE;

        auto info_line = res[current_contract_index + 1];
        auto bytecode = base::fromHex<base::Bytes>(res[current_contract_index + 3]);

        auto info = parseInfoLine(info_line);

        contracts_byte_codes.push_back({std::get<1>(info), std::move(bytecode)});
    }

    return contracts_byte_codes;
}


std::vector<std::pair<std::string, base::PropertyTree>> Solc::call_metadata_command(
  const std::string& path_to_solidity_code) const
{
    std::vector<std::string> args{"--metadata", path_to_solidity_code};

    auto res = call_command(args);

    std::vector<std::pair<std::string, base::PropertyTree>> contracts_metadatas;
    constexpr const size_t GROUP_SIZE = 4;
    for(std::size_t i = 0; i < res.size() / GROUP_SIZE; i++) {
        auto current_contract_index = i * GROUP_SIZE;

        auto info_line = res[current_contract_index + 1];
        auto metadata = base::parseJson(res[current_contract_index + 3]);

        auto info = parseInfoLine(info_line);

        contracts_metadatas.push_back({std::get<1>(info), std::move(metadata)});
    }

    return contracts_metadatas;
}

} // namespace vm