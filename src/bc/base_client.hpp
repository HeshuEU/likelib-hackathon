#pragma once

#include "address.hpp"
#include "types.hpp"

#include <string>

namespace bc
{

class BaseClient
{
  public:
    /// Constructor that create lazy(connect will be established at call method) chanel to specified ip address
    /// \param connect_address ip:port
    virtual ~BaseClient() = default;

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
    /// \return hash of transaction
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    virtual std::string transaction(
        bc::Balance amount, const bc::Address& from_address, const bc::Address& to_address) = 0;

    /// method call remote server method(specified ip address in constructor) with similar params
    /// \param test_request sha256 from secret data request
    /// \return sha256 from secret data response
    /// \throw base::Error if call was with not ok grpc status(Networks errors, serialization error and
    /// exception during processing on server instance)
    virtual std::string test(const std::string& test_request) = 0;
};

} // namespace bc