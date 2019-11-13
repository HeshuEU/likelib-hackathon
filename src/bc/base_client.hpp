#pragma once

#include "address.hpp"
#include "types.hpp"

#include <string>

namespace bc {

    class BaseClient {
    public:
        virtual ~BaseClient() = default;

        virtual bc::Balance balance(const bc::Address &address) = 0;

        virtual std::string transaction(bc::Balance amount, const bc::Address &from_address,
                                        const bc::Address &to_address) = 0;
    };

} // namespace rpc