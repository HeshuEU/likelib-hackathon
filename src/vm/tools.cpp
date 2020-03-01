#include "tools.hpp"
#include "error.hpp"

#include <algorithm>

namespace vm
{


namespace detail
{
    template<typename N>
    base::Bytes encode(N value)
    {
        if(sizeof(value) > 32) {
            RAISE_ERROR(base::InvalidArgument, "given type more than 32 bytes");
        }
        base::Bytes real(sizeof(value));

        memcpy(real.toArray(), &value, sizeof(value));

        std::reverse(real.toArray(), real.toArray() + real.size());
        return base::Bytes(32 - sizeof(value)) + real;
    }

} // namespace detail


base::Bytes copy(const uint8_t* t, size_t t_size)
{
    base::Bytes res(t_size);
    memcpy(res.toArray(), t, t_size);
    return res;
}


base::Bytes toBytes(const evmc::address& addr)
{
    return copy(addr.bytes, 20);
}


evmc::address toAddress(const base::Bytes& data)
{
    if(data.size() != 20) {
        RAISE_ERROR(base::InvalidArgument, "data len is not 20 bytes");
    }
    evmc::address res;
    memcpy(res.bytes, data.toArray(), 20);
    return res;
}


base::Bytes toBytes(const evmc::bytes32& bytes)
{
    return copy(bytes.bytes, 32);
}


evmc::bytes32 toEvmcBytes32(const base::Bytes& data)
{
    if(data.size() != 32) {
        RAISE_ERROR(base::InvalidArgument, "data len is not 32 bytes");
    }
    evmc::bytes32 res;
    memcpy(res.bytes, data.toArray(), 32);
    return res;
}


bc::Balance toBalance(evmc_uint256be value)
{
    auto val = toBytes(value).toHex();
    char* end;
    return std::strtoull(val.c_str(), &end, 16);
}


evmc_uint256be toEvmcUint256(const bc::Balance& balance)
{
    auto bytes = detail::encode(balance);
    evmc_uint256be res;

    memcpy(res.bytes, bytes.toArray(), bytes.size());

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
    if(result_size % 32 != 0) {
        result_size = ((result_size / 32) + 1) * 32;
    }

    base::Bytes res(result_size);
    memcpy(res.toArray(), str.data(), str.size());

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
    if(data.size() % 32 != 0) {
        RAISE_ERROR(base::InvalidArgument, "data not equal 32 bytes");
    }

    auto real_part = data.takePart(data.size() - sizeof(size_t), data.size());
    std::reverse(real_part.toArray(), real_part.toArray() + real_part.size());
    size_t value;
    memcpy(&value, real_part.toArray(), real_part.size());
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


bc::Address toNativeAddress(const evmc::address& addr)
{
    base::Bytes raw_address(addr.bytes, bc::Address::BYTE_LENGTH);
    bc::Address address(raw_address);
    return address;
}


evmc::address toEthAddress(const bc::Address& address)
{
    evmc::address ret;
    auto byte_address = address.getBytes();
    ASSERT(byte_address.size() == std::size(ret.bytes));
    std::copy(byte_address.toArray(), byte_address.toArray() + byte_address.size(), ret.bytes);
    return ret;
}


} // namespace vm