#include "address.hpp"

namespace bc {

    Address::Address(const char *address) {
        _address = std::string(address);
    }

    std::string Address::toString() const {
        return "";
    }
}