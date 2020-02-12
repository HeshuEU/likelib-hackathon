#include "messages.hpp"

#include <base/assert.hpp>

#include <boost/process.hpp>

#include <set>


namespace bp = ::boost::process;

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


Solc::Solc() : _path_to_solc{bp::search_path(_SOLC_NAME)}
{
    if(!boost::filesystem::exists(_path_to_solc)) {
        RAISE_ERROR(base::InaccessibleFile, "Solidity compiler was not found");
    }
}


std::optional<Contracts> Solc::compile(const std::string& path_to_solidity_code) const
{
    auto full_compilation_output = call_full_compilation_command(path_to_solidity_code);
    auto runtime_compilation_output = call_runtime_compilation_command(path_to_solidity_code);
    auto abi_output = call_abi_command(path_to_solidity_code);
    auto hashes_output = call_hashes_command(path_to_solidity_code);
    auto metadata_output = call_metadata_command(path_to_solidity_code);

    Contracts contracts{};
    for(const auto& contract_name_full_code_pair: full_compilation_output) {
        CompiledContract contract;
        contract.setName(contract_name_full_code_pair.first);
        contract.setFullCode(contract_name_full_code_pair.second);

        for(const auto& contract_name_code_pair: runtime_compilation_output) {
            if(contract_name_code_pair.first == contract.getName()) {
                contract.setRuntimeCode(contract_name_code_pair.second);
            }
        }

        for(const auto& contract_name_abi_pair: abi_output) {
            if(contract_name_abi_pair.first == contract.getName()) {
                contract.setAbiSpecification(contract_name_abi_pair.second);
            }
        }

        for(const auto& contract_name_hashes_pair: hashes_output) {
            if(contract_name_hashes_pair.first == contract.getName()) {
                contract.setSignatures(contract_name_hashes_pair.second);
            }
        }

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
    while(out) {
        std::string out_result;
        out >> out_result;
        out_put_result_values.push_back(out_result);
    }

    if(c.exit_code()) {
        std::ostringstream s;
        for(const auto& item: out_put_result_values) {
            s << item;
        }
        RAISE_ERROR(base::SystemCallFailed, s.str());
    }

    return out_put_result_values;
}


std::vector<std::pair<std::string, base::Bytes>> Solc::call_runtime_compilation_command(
    const std::string& path_to_solidity_code) const
{
    std::vector<std::string> args;
    args.push_back("--bin-runtime");
    args.push_back(path_to_solidity_code);

    auto res = call_command(args);

    static const std::set<std::string> ignore{"=======", "Binary", "of", "the", "runtime", "part:"};
    std::vector<std::string> out_put_result_values;
    for(const auto& item: res) {
        if(ignore.find(item) == ignore.end()) {
            out_put_result_values.push_back(item);
        }
    }

    std::vector<std::pair<std::string, base::Bytes>> contracts_byte_codes;
    for(std::size_t i = 0; i < out_put_result_values.size() / 2; i++) {
        auto current_contract_path_index = i * 2;
        auto current_data_index = current_contract_path_index + 1;
        auto path = out_put_result_values[current_contract_path_index];
        auto delimiter_pos = path.find(':');
        auto source_file = path.substr(0, delimiter_pos);
        auto contract_name = path.substr(delimiter_pos + 1, path.size());
        auto bytecode = base::Bytes::fromHex(out_put_result_values[current_data_index]);
        contracts_byte_codes.push_back({std::move(contract_name), std::move(bytecode)});
    }

    return contracts_byte_codes;
}


std::vector<std::pair<std::string, base::Bytes>> Solc::call_full_compilation_command(
    const std::string& path_to_solidity_code) const
{
    std::vector<std::string> args;
    args.push_back("--bin");
    args.push_back(path_to_solidity_code);

    auto res = call_command(args);

    static const std::set<std::string> ignore{"=======", "Binary:"};
    std::vector<std::string> out_put_result_values;
    for(const auto& item: res) {
        if(ignore.find(item) == ignore.end()) {
            out_put_result_values.push_back(item);
        }
    }

    std::vector<std::pair<std::string, base::Bytes>> contracts_byte_codes;
    for(std::size_t i = 0; i < out_put_result_values.size() / 2; i++) {
        auto current_contract_path_index = i * 2;
        auto current_data_index = current_contract_path_index + 1;
        auto path = out_put_result_values[current_contract_path_index];
        auto delimiter_pos = path.find(':');
        auto source_file = path.substr(0, delimiter_pos);
        auto contract_name = path.substr(delimiter_pos + 1, path.size());
        auto bytecode = base::Bytes::fromHex(out_put_result_values[current_data_index]);
        contracts_byte_codes.push_back({std::move(contract_name), std::move(bytecode)});
    }

    return contracts_byte_codes;
}


std::vector<std::pair<std::string, base::PropertyTree>> Solc::call_metadata_command(
    const std::string& path_to_solidity_code) const
{
    std::vector<std::string> args;
    args.push_back("--metadata");
    args.push_back(path_to_solidity_code);

    auto res = call_command(args);

    static const std::set<std::string> ignore{"=======", "Metadata:"};
    std::vector<std::string> out_put_result_values;
    for(const auto& item: res) {
        if(ignore.find(item) == ignore.end()) {
            out_put_result_values.push_back(item);
        }
    }

    std::vector<std::pair<std::string, base::PropertyTree>> contracts_metadatas;
    for(std::size_t i = 0; i < out_put_result_values.size() / 2; i++) {
        auto current_contract_path_index = i * 2;
        auto current_data_index = current_contract_path_index + 1;
        auto path = out_put_result_values[current_contract_path_index];
        auto delimiter_pos = path.find(':');
        auto source_file = path.substr(0, delimiter_pos);
        auto contract_name = path.substr(delimiter_pos + 1, path.size());

        auto metadata = base::parseJson(out_put_result_values[current_data_index]);

        contracts_metadatas.push_back({std::move(contract_name), std::move(metadata)});
    }

    return contracts_metadatas;
}


std::vector<std::pair<std::string, base::PropertyTree>> Solc::call_abi_command(
    const std::string& path_to_solidity_code) const
{
    std::vector<std::string> args;
    args.push_back("--abi");
    args.push_back(path_to_solidity_code);

    auto res = call_command(args);

    static const std::set<std::string> ignore{"=======", "Contract", "JSON", "ABI"};
    std::vector<std::string> out_put_result_values;
    for(const auto& item: res) {
        if(ignore.find(item) == ignore.end()) {
            out_put_result_values.push_back(item);
        }
    }

    std::vector<std::pair<std::string, base::PropertyTree>> contracts_abi_data;
    for(std::size_t i = 0; i < out_put_result_values.size() / 2; i++) {
        auto current_contract_path_index = i * 2;
        auto current_data_index = current_contract_path_index + 1;
        auto path = out_put_result_values[current_contract_path_index];
        auto delimiter_pos = path.find(':');
        auto source_file = path.substr(0, delimiter_pos);
        auto contract_name = path.substr(delimiter_pos + 1, path.size());

        auto metadata = base::parseJson(out_put_result_values[current_data_index]);

        contracts_abi_data.push_back({std::move(contract_name), std::move(metadata)});
    }

    return contracts_abi_data;
}


std::vector<std::pair<std::string, std::vector<std::pair<base::Bytes, std::string>>>> Solc::call_hashes_command(
    const std::string& path_to_solidity_code) const
{
    std::vector<std::string> args;
    args.push_back("--hashes");
    args.push_back(path_to_solidity_code);

    auto res = call_command(args);

    static const std::set<std::string> ignore{"=======", "Function", "signatures:", ""};
    std::vector<std::string> out_put_result_values;
    for(const auto& item: res) {
        if(ignore.find(item) == ignore.end()) {
            out_put_result_values.push_back(item);
        }
    }

    std::vector<std::pair<std::string, std::vector<std::pair<base::Bytes, std::string>>>> hashes;
    std::string current_contract_name;
    std::vector<std::pair<base::Bytes, std::string>> current_contract_members;
    for(size_t i = 0; i < out_put_result_values.size();) {
        if(out_put_result_values[i].find(".sol:") != std::string::npos) {
            if(!current_contract_name.empty()) {
                hashes.push_back({current_contract_name, current_contract_members});
                current_contract_name.clear();
                current_contract_members.clear();
            }
            auto path = out_put_result_values[i];
            auto delimiter_pos = path.find(':');
            auto source_file = path.substr(0, delimiter_pos);
            current_contract_name = path.substr(delimiter_pos + 1, path.size());
            i++;
        }
        auto hash = base::Bytes::fromHex(out_put_result_values[i].substr(0, out_put_result_values[i].size() - 1));
        i++;
        auto signature = out_put_result_values[i];
        i++;
        current_contract_members.push_back({hash, signature});
    }

    return hashes;
}

} // namespace vm