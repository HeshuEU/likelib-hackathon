#include "tools.hpp"

#include "vm/encode_decode.hpp"
#include "vm/error.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/foreach.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/process.hpp>
#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <filesystem>
#include <regex>

namespace bp = ::boost::process;

namespace
{
std::string readAllFile(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        RAISE_ERROR(base::InvalidArgument, "the file with this path does not exist");
    }
    std::ifstream file(path, std::ifstream::binary);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
    return buffer;
}


std::pair<base::Bytes, std::string> loadContractData(const std::filesystem::path& path)
{
    auto compiled_file_path = path / std::filesystem::path("compiled_code.bin");
    if (!std::filesystem::exists(compiled_file_path)) {
        RAISE_ERROR(base::InvalidArgument, "Contract file not exists");
    }
    std::ifstream compiled_file(compiled_file_path, std::ios::binary);
    base::Bytes bytecode(std::vector<base::Byte>(std::istreambuf_iterator<char>(compiled_file), {}));

    auto contract_metadata = path / std::filesystem::path("metadata.json");
    if (!std::filesystem::exists(contract_metadata)) {
        RAISE_ERROR(base::InvalidArgument, "Contract metadata file not exists");
    }
    auto metadata = readAllFile(contract_metadata);
    return { bytecode, metadata };
}


std::pair<std::string, std::string> parseCall(const std::string& call_string)
{
    auto first_bracket_pos = call_string.find("(");
    if (first_bracket_pos == std::string::npos || call_string[call_string.size() - 1] != ')') {
        RAISE_ERROR(base::InvalidArgument, "Wrong string for encode");
    }
    auto method = call_string.substr(0, first_bracket_pos);
    auto arguments = "[" + call_string.substr(first_bracket_pos + 1, call_string.size() - first_bracket_pos - 2) + "]";
    return { method, arguments };
}
}

namespace vm
{

namespace detail
{

template<typename N>
base::Bytes encode(N value)
{
    if (sizeof(value) > 32) {
        RAISE_ERROR(base::InvalidArgument, "given type more than 32 bytes");
    }
    base::Bytes real(sizeof(value));

    memcpy(real.getData(), &value, sizeof(value));

    std::reverse(real.getData(), real.getData() + real.size());
    return base::Bytes(32 - sizeof(value)) + real;
}


base::Bytes encode(lk::Balance value)
{
    base::Bytes ret(32);
    for (std::size_t i = 1; i <= ret.size(); ++i) {
        ret[ret.size() - i] = (static_cast<base::Byte>(std::atoi((value % 256).toString().c_str()))); // TODO: rewrite
        value /= 256;
    }
    return ret;
}

} // namespace detail


base::Bytes copy(const uint8_t* t, size_t t_size)
{
    base::Bytes res(t_size);
    memcpy(res.getData(), t, t_size);
    return res;
}


base::Bytes toBytes(const evmc::address& addr)
{
    return copy(addr.bytes, 20);
}


evmc::address toAddress(const base::Bytes& data)
{
    if (data.size() != 20) {
        RAISE_ERROR(base::InvalidArgument, "data len is not 20 bytes");
    }
    evmc::address res;
    memcpy(res.bytes, data.getData(), 20);
    return res;
}


base::Bytes toBytes(const evmc::bytes32& bytes)
{
    return copy(bytes.bytes, 32);
}


evmc::bytes32 toEvmcBytes32(const base::Bytes& data)
{
    if (data.size() != 32) {
        RAISE_ERROR(base::InvalidArgument, "data len is not 32 bytes");
    }
    evmc::bytes32 res;
    memcpy(res.bytes, data.getData(), 32);
    return res;
}


evmc::bytes32 toEvmcBytes32(const base::FixedBytes<32>& data)
{
    evmc::bytes32 res;
    memcpy(res.bytes, data.getData(), 32);
    return res;
}


lk::Balance toBalance(evmc_uint256be value)
{
    auto val = base::toHex<base::Bytes>(toBytes(value));
    char* end;
    return std::strtoull(val.c_str(), &end, 16);
}


evmc_uint256be toEvmcUint256(const lk::Balance& balance)
{
    auto bytes = detail::encode(balance);
    evmc_uint256be res;

    memcpy(res.bytes, bytes.getData(), bytes.size());
    return res;
}


std::string getStringArg(size_t position, const base::Bytes& data)
{
    auto offset_data_start = position * 32;
    auto offset_data_end = offset_data_start + 32;
    auto offset = decodeAsSizeT(data.takePart(offset_data_start, offset_data_end));

    return decodeAsString(data.takePart(offset, data.size()));
}


base::Bytes encode(const std::string& str)
{
    auto str_len = encode(str.size());

    auto result_size = str.size();
    if (result_size % 32 != 0) {
        result_size = ((result_size / 32) + 1) * 32;
    }

    base::Bytes res(result_size);
    memcpy(res.getData(), str.data(), str.size());

    return str_len + res;
}


std::string decodeAsString(const base::Bytes& data)
{
    auto str_len_data = data.takePart(0, 32);
    auto str_len = decodeAsSizeT(str_len_data);

    auto str_data = data.takePart(32, 32 + str_len);
    return str_data.toString();
}


base::Bytes encode(size_t value)
{
    return detail::encode(value);
}


size_t decodeAsSizeT(const base::Bytes& data)
{
    if (data.size() % 32 != 0) {
        RAISE_ERROR(base::InvalidArgument, "data not equal 32 bytes");
    }

    auto real_part = data.takePart(data.size() - sizeof(size_t), data.size());
    std::reverse(real_part.getData(), real_part.getData() + real_part.size());
    size_t value;
    memcpy(&value, real_part.getData(), real_part.size());
    return value;
}


base::Bytes encode(uint32_t value)
{
    return detail::encode(value);
}


base::Bytes encode(uint16_t value)
{
    return detail::encode(value);
}


base::Bytes encode(uint8_t value)
{
    return detail::encode(value);
}


lk::Address toNativeAddress(const evmc::address& addr)
{
    base::Bytes raw_address(addr.bytes, lk::Address::LENGTH_IN_BYTES);
    lk::Address address(raw_address);
    return address;
}


evmc::address toEthAddress(const lk::Address& address)
{
    evmc::address ret;
    auto byte_address = address.getBytes();
    ASSERT(byte_address.size() == std::size(ret.bytes));
    std::copy(byte_address.getData(), byte_address.getData() + byte_address.size(), ret.bytes);
    return ret;
}


base::Keccak256 methodHash(const boost::property_tree::ptree& method_abi)
{
    //    auto ser = base::PropertyTree(method_abi).toString();
    std::string method =
      method_abi.get<std::string>("type") == "function" ? method_abi.get<std::string>("name") + '(' : "constructor(";

    BOOST_FOREACH (const auto& argument, method_abi.get_child("inputs")) {
        method += argument.second.get<std::string>("type") + ',';
    }
    if (method[method.size() - 1] == ',') {
        method.erase(method.size() - 1, 1);
    }
    method += ')';
    return base::Keccak256::compute(base::Bytes(method));
}


namespace
{

std::string replaceAddressConstructor(const std::string& call)
{
    std::regex reg("((\\(|,|\\s)Address\\((\\w+)\\)(\\)|,|\\s))");
    auto words_begin = std::sregex_iterator(call.begin(), call.end(), reg);
    auto words_end = std::sregex_iterator();
    std::string result = call;
    for (auto i = words_begin; i != words_end; i++) {
        std::string matched_world{ (*i)[1] };

        auto constructor = matched_world.substr(1, matched_world.size() - 2);

        constexpr const std::size_t begin_cut = sizeof("Address");
        auto base58_address = constructor.substr(begin_cut, constructor.size() - begin_cut - 1);

        auto decoded_address = base::base58Decode(base58_address);
        base::Bytes prefix(32 - decoded_address.size());
        auto hex_view_address = base::toHex(prefix + decoded_address);

        result =
          std::regex_replace(result, std::regex("Address\\(" + base58_address + "\\)"), "\"" + hex_view_address + "\"");
    }
    return result;
}

}


std::optional<base::Bytes> encodeCall(const std::filesystem::path& path_to_code_folder, const std::string& call)
{
    auto formatted_call = replaceAddressConstructor(call);
    auto tmp = vm::encodeMessage(path_to_code_folder, formatted_call);
    return base::fromHex<base::Bytes>(tmp);
}


std::optional<std::string> decodeOutput(const std::filesystem::path& path_to_code_folder,
                                        const std::string& method,
                                        const std::string& output)
{
    return vm::decodeMessage(path_to_code_folder, method, output);
}


namespace
{

constexpr const char* PATH_VARIABLE_NAME = "PATH";
constexpr const char* PATH_DELIMITER_NAME = ":";
constexpr const char* PYTHON_EXEC = "python3.7";

std::optional<std::string> findPython()
{
    std::string path_var = std::getenv(PATH_VARIABLE_NAME);
    std::vector<std::string> results;

    boost::algorithm::split(results, path_var, boost::is_any_of(PATH_DELIMITER_NAME));

    for (const auto& item : results) {
        auto current = std::filesystem::path(item) / std::filesystem::path(PYTHON_EXEC);
        if (std::filesystem::exists(current)) {
            return current;
        }
    }
    return std::nullopt;
}

} // namespace


std::string callPython(std::vector<std::string>& args)
{
    bp::ipstream out;
    int exit_code = 2;

    auto python_exec = findPython();
    if (python_exec) {
        try {
            bp::child c(python_exec.value(), args, bp::std_out > out);
            c.wait();
            exit_code = c.exit_code();
        }
        catch (const std::exception& e) {
            RAISE_ERROR(base::SystemCallFailed, e.what());
        }
    }
    else {
        RAISE_ERROR(base::InaccessibleFile, "can't find python executable file");
    }

    std::ostringstream s;
    s << out.rdbuf();

    if (exit_code) {
        RAISE_ERROR(base::SystemCallFailed, s.str());
    }
    return s.str();
}


std::string encodeMessage(const std::string& contract_path, const std::string& data)
{
    auto status = PyImport_AppendInittab("encode_decode", PyInit_encode_decode);
    if (status == -1) {
        RAISE_ERROR(base::RuntimeError, "Decode python error");
    }
    Py_Initialize();
    auto module = PyImport_ImportModule("encode_decode");
    if (module == nullptr) {
        Py_Finalize();
        RAISE_ERROR(base::RuntimeError, "Decode python error");
    }

    auto [bytecode, metadata] = loadContractData(contract_path);
    auto [method, arguments] = parseCall(data);
    auto encode_result =
      method == "constructor" ?
        ::encodeMessageConstructor(arguments.c_str(), bytecode.toString().c_str(), metadata.c_str()) :
        ::encodeMessageFunction(method.c_str(), arguments.c_str(), bytecode.toString().c_str(), metadata.c_str());

    Py_Finalize();
    return encode_result.substr(2, encode_result.size() - 2);
}


std::string decodeMessage(const std::string& contract_path, const std::string& method, const std::string& data)
{
    auto status = PyImport_AppendInittab("encode_decode", PyInit_encode_decode);
    if (status == -1) {
        RAISE_ERROR(base::RuntimeError, "Decode python error");
    }
    Py_Initialize();
    auto module = PyImport_ImportModule("encode_decode");
    if (module == nullptr) {
        Py_Finalize();
        RAISE_ERROR(base::RuntimeError, "Decode python error");
    }

    auto [bytecode, metadata] = loadContractData(contract_path);
    auto decode_result = ::decodeMessage(metadata.c_str(), method.c_str(), data.c_str());
    Py_Finalize();
    return decode_result;
}

} // namespace vm