#include "account_manager.hpp"

#include "base/error.hpp"

namespace lk
{


std::uint64_t AccountState::getNonce() const noexcept
{
    return _nonce;
}


bc::Balance AccountState::getBalance() const noexcept
{
    return _balance;
}


void AccountState::setBalance(bc::Balance new_balance)
{
    _balance = new_balance;
}



void AccountState::addBalance(bc::Balance delta)
{
    _balance += delta;
}


void AccountState::subBalance(bc::Balance delta)
{
    if(_balance < delta) {
        throw base::LogicError("trying to take more LK from account than it has");
    }
    _balance -= delta;
}


const base::Bytes& AccountState::getCode() const noexcept
{
    return _code;
}


bool AccountState::checkStorageValue(const base::Sha256& key) const
{
    return _storage.find(key) != _storage.end();
}


std::optional<std::reference_wrapper<const base::Bytes>> AccountState::getStorageValue(const base::Sha256& key) const
{
    if(auto it = _storage.find(key); it == _storage.end()) {
        return std::nullopt;
    }
    else {
        return it->second;
    }
}


void AccountState::setStorageValue(const base::Sha256& key, base::Bytes value)
{
    _storage[key] = std::move(value);
}


bc::Balance AccountManager::getBalance(const bc::Address& address) const
{
    LOG_CURRENT_FUNCTION << "with address = " << address;
    LOG_CURRENT_FUNCTION << "acquiring shared lock";
    std::shared_lock lk(_rw_mutex);
    auto it = _states.find(address);
    if(it == _states.end()) {
        LOG_CURRENT_FUNCTION << "address not found, return 0";
        return bc::Balance{0};
    }
    else {
        LOG_CURRENT_FUNCTION << "return " << it->second.getBalance();
        return it->second.getBalance();
    }
}


bool AccountManager::checkTransaction(const bc::Transaction& tx) const
{
    std::shared_lock lk(_rw_mutex);
    if(_states.find(tx.getFrom()) == _states.end()) {
        return false;
    }
    return getBalance(tx.getFrom()) >= tx.getAmount();
}


void AccountManager::update(const bc::Transaction& tx)
{
    std::unique_lock lk(_rw_mutex);
    auto from_iter = _states.find(tx.getFrom());

    if(from_iter == _states.end() || from_iter->second.getBalance() < tx.getAmount()) {
        RAISE_ERROR(base::LogicError, "account doesn't have enough funds to perform the operation");
    }

    auto& from_state = from_iter->second;
    from_state.subBalance(tx.getAmount());
    if(auto to_iter = _states.find(tx.getTo()); to_iter != _states.end()) {
        to_iter->second.addBalance(tx.getAmount());
    }
    else {
        AccountState to_state;
        to_state.setBalance(tx.getAmount());
        _states.insert({tx.getTo(), std::move(to_state)});
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
        AccountState state;
        state.setBalance(tx.getAmount());
        _states.insert({tx.getTo(), std::move(state)});
    }
}

} // namespace lk
