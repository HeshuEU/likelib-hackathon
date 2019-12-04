#include "balance_manager.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

namespace bc
{

BalanceManager::BalanceManager(const TransactionsSet& tx_set)
{
    for(auto tx: tx_set) {
        auto it = _storage.find(tx.getFrom());
        if(it == _storage.end()) {
            _storage[tx.getTo()] += tx.getAmount();
        }
        else {
            _storage[tx.getTo()] = tx.getAmount();
        }
    }
}


Balance BalanceManager::getBalance(const Address& address) const
{
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
    ASSERT(getBalance(tx.getFrom()) >= tx.getAmount());

    auto from_iter = _storage.find(tx.getFrom());

    from_iter->second = from_iter->second - tx.getAmount();

    if(auto to_iter = _storage.find(tx.getTo()); to_iter != _storage.end()) {
        to_iter->second += tx.getAmount();
    }
    else {
        _storage[tx.getTo()] = tx.getAmount();
    }
}

} // namespace bc