#include "tools.hpp"
#include "error.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/process.hpp>
#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <regex>

namespace bp = ::boost::process;

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
    auto exec_script = std::filesystem::current_path() / std::filesystem::path{ "encoder.py" };
    if (std::filesystem::exists(exec_script)) {
        auto formatted_call = replaceAddressConstructor(call);
        std::vector<std::string> args{ exec_script, "--contract_path", path_to_code_folder, "--call", formatted_call };
        std::istringstream iss(callPython(args));
        std::string tmp;
        iss >> tmp;
        return base::fromHex<base::Bytes>(tmp);
    }
    return std::nullopt;
}


std::optional<std::string> decodeOutput(const std::filesystem::path& path_to_code_folder,
                                        const std::string& method,
                                        const std::string& output)
{
    auto exec_script = std::filesystem::current_path() / std::filesystem::path{ "decoder.py" };
    if (std::filesystem::exists(exec_script)) {
        std::vector<std::string> args{
            exec_script, "--contract_path", path_to_code_folder, "--method", method, "--data", output
        };
        return callPython(args);
    }
    return std::nullopt;
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


} // namespace vm