#include "account_manager.hpp"

#include "base/error.hpp"

namespace lk
{

AccountManager::AccountManager(const std::map<bc::Address, bc::Balance>& initial_state) : _storage(initial_state)
{}


bc::Balance AccountManager::getBalance(const bc::Address& address) const
{
    LOG_CURRENT_FUNCTION << "with address = " << address;
    LOG_CURRENT_FUNCTION << "acquiring shared lock";
    std::shared_lock lk(_rw_mutex);
    auto it = _storage.find(address);
    if(it == _storage.end()) {
        LOG_CURRENT_FUNCTION << "address not found, return 0";
        return bc::Balance{0};
    }
    else {
        LOG_CURRENT_FUNCTION << "return " << it->second;
        return it->second;
    }
}


bool AccountManager::checkTransaction(const bc::Transaction& tx) const
{
    std::shared_lock lk(_rw_mutex);
    if(_storage.find(tx.getFrom()) == _storage.end()) {
        return false;
    }
    return getBalance(tx.getFrom()) >= tx.getAmount();
}


void AccountManager::update(const bc::Transaction& tx)
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


void AccountManager::update(const bc::Block& block)
{
    for(const auto& tx: block.getTransactions()) {
        update(tx);
    }
}


void AccountManager::updateFromGenesis(const bc::Block& block)
{
    std::unique_lock lk(_rw_mutex);
    for(const auto& tx: block.getTransactions()) {
        _storage[tx.getTo()] = tx.getAmount();
    }
}

} // namespace lk
