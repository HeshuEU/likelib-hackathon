#include "balance_manager.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

namespace bc
{

BalanceManager::BalanceManager(const std::map<Address, Balance>& initial_state) : _storage(initial_state)
{}


Balance BalanceManager::getBalance(const Address& address) const
{
    if(address == bc::BASE_ADDRESS) {
        return std::numeric_limits<Balance>::max();
    }

    auto it = _storage.find(address);
    if(it == _storage.end()) {
        return Balance{0};
    }
    else {
        return it->second;
    }
}


bool BalanceManager::checkTransaction(const Transaction& tx)
{
    return getBalance(tx.getFrom()) >= tx.getAmount();
}


void BalanceManager::update(const Transaction& tx)
{
    auto from_iter = _storage.find(tx.getFrom());
    ASSERT(getBalance(tx.getFrom()) >= tx.getAmount());
    from_iter->second = from_iter->second - tx.getAmount();

    if(auto to_iter = _storage.find(tx.getTo()); to_iter != _storage.end()) {
        to_iter->second += tx.getAmount();
    }
    else {
        _storage[tx.getTo()] = tx.getAmount();
    }
}

} // namespace bc