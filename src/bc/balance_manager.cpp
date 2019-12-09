#include "balance_manager.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

namespace bc
{

BalanceManager::BalanceManager(const std::map<Address, Balance>& initial_state) : _storage(initial_state)
{}


Balance BalanceManager::getBalance(const Address& address) const
{
    std::shared_lock lk(_rw_mutex);
    auto it = _storage.find(address);
    if(it == _storage.end()) {
        return Balance{0};
    }
    else {
        return it->second;
    }
}


bool BalanceManager::checkTransaction(const Transaction& tx) const
{

    return _storage.find(tx.getFrom()) != _storage.end() && getBalance(tx.getFrom()) >= tx.getAmount();
}


void BalanceManager::update(const Transaction& tx)
{
    std::unique_lock lk(_rw_mutex);
    auto from_iter = _storage.find(tx.getFrom());

    if(from_iter == _storage.end()) {
        RAISE_ERROR(base::LogicError, "account doesn't have entry in balances db");
    }
    else if(from_iter->second < tx.getAmount()) {
        RAISE_ERROR(base::LogicError, "account doesn't have enough funds to perform the operation");
    }

    from_iter->second = from_iter->second - tx.getAmount();
    if(auto to_iter = _storage.find(tx.getTo()); to_iter != _storage.end()) {
        to_iter->second += tx.getAmount();
    }
    else {
        _storage[tx.getTo()] = tx.getAmount();
    }
}


void BalanceManager::update(const Block& block)
{
    for(const auto& tx : block.getTransactions()) {
        update(tx);
    }
}


void BalanceManager::updateFromGenesis(const bc::Block &block) {
    std::unique_lock lk(_rw_mutex);
    for(const auto& tx : block.getTransactions()) {
        _storage[tx.getTo()] = tx.getAmount();
    }
}

} // namespace bc