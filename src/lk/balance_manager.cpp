#include "balance_manager.hpp"

#include "base/error.hpp"

namespace lk
{

BalanceManager::BalanceManager(const std::map<bc::Address, bc::Balance>& initial_state) : _storage(initial_state)
{}


bc::Balance BalanceManager::getBalance(const bc::Address& address) const
{
    std::shared_lock lk(_rw_mutex);
    auto it = _storage.find(address);
    if(it == _storage.end()) {
        return bc::Balance{0};
    }
    else {
        return it->second;
    }
}


bool BalanceManager::checkTransaction(const bc::Transaction& tx) const
{
    return getBalance(tx.getFrom()) >= tx.getAmount();
}


void BalanceManager::update(const bc::Transaction& tx)
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


void BalanceManager::update(const bc::Block& block)
{
    for(const auto& tx: block.getTransactions()) {
        update(tx);
    }
}


void BalanceManager::updateFromGenesis(const bc::Block& block)
{
    std::unique_lock lk(_rw_mutex);
    for(const auto& tx: block.getTransactions()) {
        _storage[tx.getTo()] = tx.getAmount();
    }
}

} // namespace lk
