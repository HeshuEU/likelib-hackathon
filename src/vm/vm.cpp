#include "vm.hpp"

#include "vm/error.hpp"

#include "base/log.hpp"

#include <evmc/loader.h>

#include <boost/process.hpp>

#include <filesystem>
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


std::vector<std::string> callCommand(const boost::filesystem::path& path_to_solc, const std::vector<std::string>& args)
{
    bp::ipstream out;
    bp::child c(path_to_solc, args, bp::std_out > out);

    std::vector<std::string> out_put_result_values;
    std::string out_result;
    while (c.running() && std::getline(out, out_result)) {
        out_put_result_values.push_back(out_result);
    }
    c.wait();

    if (c.exit_code()) {
        std::ostringstream s;
        for (const auto& item : out_put_result_values) {
            s << item;
        }
        RAISE_ERROR(base::SystemCallFailed, s.str());
    }

    return out_put_result_values;
}


std::vector<std::pair<std::string, base::Bytes>> callCompilationCommand(const boost::filesystem::path& path_to_solc,
                                                                        const std::string& path_to_solidity_file)
{
    std::vector<std::string> args{ "--bin", path_to_solidity_file };
    auto res = callCommand(path_to_solc, args);

    std::vector<std::pair<std::string, base::Bytes>> contracts_byte_codes;
    constexpr const size_t GROUP_SIZE = 4;
    for (std::size_t i = 0; i < res.size() / GROUP_SIZE; i++) {
        auto current_contract_index = i * GROUP_SIZE;

        auto info_line = res[current_contract_index + 1];
        auto bytecode = base::fromHex<base::Bytes>(res[current_contract_index + 3]);

        auto info = parseInfoLine(info_line);

        contracts_byte_codes.push_back({ std::get<1>(info), std::move(bytecode) });
    }

    return contracts_byte_codes;
}


std::vector<std::pair<std::string, base::PropertyTree>> callMetadataCommand(const boost::filesystem::path& path_to_solc,
                                                                            const std::string& path_to_solidity_file)
{
    std::vector<std::string> args{ "--metadata", path_to_solidity_file };

    auto res = callCommand(path_to_solc, args);

    std::vector<std::pair<std::string, base::PropertyTree>> contracts_metadatas;
    constexpr const size_t GROUP_SIZE = 4;
    for (std::size_t i = 0; i < res.size() / GROUP_SIZE; i++) {
        auto current_contract_index = i * GROUP_SIZE;

        auto info_line = res[current_contract_index + 1];
        auto metadata = base::parseJson(res[current_contract_index + 3]);

        auto info = parseInfoLine(info_line);

        contracts_metadatas.push_back({ std::get<1>(info), std::move(metadata) });
    }

    return contracts_metadatas;
}


std::string compilerName()
{
    static const std::string_view SOLC_NAME = "solc";
    return std::string{ SOLC_NAME };
}


std::filesystem::path getVmPath()
{
    static const std::filesystem::path lib_name = std::filesystem::absolute("libevmone.so.0.4");

    if (std::filesystem::exists(lib_name)) {
        if (std::filesystem::is_symlink(lib_name)) {
            std::error_code ec;
            auto result = std::filesystem::read_symlink(lib_name, ec);
            if (!ec) {
                RAISE_ERROR(base::InaccessibleFile, "Vm library was not found");
            }
            return std::filesystem::absolute(result);
        }
        else {
            return std::filesystem::absolute(lib_name);
        }
    }
    else {
        RAISE_ERROR(base::InaccessibleFile, "Vm library was not found");
    }
}

} // namespace


namespace vm
{

CompiledContract::CompiledContract(std::string _name)
  : name{ _name }
{}


std::optional<Contracts> compile(const std::string& path_to_solidity_file)
{
    boost::filesystem::path path_to_solc{ bp::search_path(compilerName()) };
    if (!boost::filesystem::exists(path_to_solc)) {
        RAISE_ERROR(base::InaccessibleFile, "Solidity compiler was not found");
    }

    auto compilation_output = callCompilationCommand(path_to_solc, path_to_solidity_file);
    auto metadata_output = callMetadataCommand(path_to_solc, path_to_solidity_file);

    Contracts contracts{};
    for (const auto& contract_name_full_code_pair : compilation_output) {
        CompiledContract contract{ contract_name_full_code_pair.first };
        contract.code = contract_name_full_code_pair.second;

        for (const auto& contract_name_metadata_pair : metadata_output) {
            if (contract_name_metadata_pair.first == contract.name) {
                contract.metadata = contract_name_metadata_pair.second;
            }
        }

        contracts.push_back(std::move(contract));
    }

    return contracts;
}


evmc::VM load()
{
    evmc_loader_error_code load_error_code;

    auto vm_ptr = evmc_load_and_create(getVmPath().c_str(), &load_error_code);

    if (load_error_code != EVMC_LOADER_SUCCESS || vm_ptr == nullptr) {
        switch (load_error_code) {
            case EVMC_LOADER_SUCCESS:
                RAISE_ERROR(base::LogicError, "Error status is success but pointer is null");
            case EVMC_LOADER_CANNOT_OPEN:
                RAISE_ERROR(base::InaccessibleFile, "File can't be open");
            case EVMC_LOADER_SYMBOL_NOT_FOUND:
                RAISE_ERROR(VmError, "Vm dll has incorrect format");
            case EVMC_LOADER_ABI_VERSION_MISMATCH:
                RAISE_ERROR(VmError, "Invalid vm abi version");
            default:
                RAISE_ERROR(VmError, "Undefined error at loading vm");
        }
    }

    evmc::VM vm_instance{ vm_ptr };
    LOG_INFO << "Created EVM name: " << vm_instance.name() << ", version:" << vm_instance.version();

    if (!vm_instance.is_abi_compatible()) {
        RAISE_ERROR(VmError, " ABI version is incompatible.");
    }

    auto vm_capabilities = vm_instance.get_capabilities();

    if (vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_EVM1) {
        LOG_INFO << "EVM compatible with EVM1 instructions";
    }

    if (vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_EWASM) {
        LOG_INFO << "EVM compatible with EWASM instructions";
    }

    if (vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_PRECOMPILES) {
        LOG_INFO << "EVM compatible with precompiles instructions";
    }

    return vm_instance;
}

} // namespace vm