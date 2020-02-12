#include "tools.hpp"
#include "error.hpp"

namespace vm
{


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


base::Bytes toBalance(evmc_uint256be value)
{
    return copy(value.bytes, 32);
}


evmc_uint256be toEvmcUint256(const base::Bytes& data)
{
    if(data.size() != 32) {
        RAISE_ERROR(base::InvalidArgument, "data len is not 32 bytes");
    }
    evmc_uint256be res;
    memcpy(res.bytes, data.toArray(), 32);
    return res;
}

} // namespace vm