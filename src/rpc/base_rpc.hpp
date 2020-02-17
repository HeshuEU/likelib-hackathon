#pragma once

#include "bc/address.hpp"
#include "bc/types.hpp"

#include "base/time.hpp"

#include <string>

namespace rpc
{

class BaseRpc
{
  public:
    /// Constructor that create lazy(connect will be established at call method) chanel to specified ip address
    /// \param connect_address ip:port
    virtual ~BaseRpc() = default;

    /// method call remote server method(specified ip address in constructor) with similar params
    /// \param address of account
    /// \return result of balance by specific address
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    virtual bc::Balance balance(const bc::Address& address) = 0;

    /// method call remote server method(specified ip address in constructor) with similar params
    /// \param amount money
    /// \param from_address
    /// \param to_address
    /// \param transaction_time time of transaction creation
    /// \param keys_path path to a directory with keys
    /// \return hash of transaction
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    /// \throw base::InaccessibleFile if private key file cannot be accessed
    virtual std::string transaction(bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address,
        const base::Time& transaction_time, const std::filesystem::path& keys_path) = 0;

    /// method call remote server method(specified ip address in constructor) with similar params
    /// \param test_request sha256 from secret data request
    /// \return sha256 from secret data response
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    virtual std::string test(const std::string& test_request) = 0;
};

} // namespace rpc