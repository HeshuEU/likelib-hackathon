#include "managers.hpp"

#include "base/error.hpp"
#include "lk/core.hpp"

namespace lk
{


std::uint64_t AccountState::getNonce() const noexcept
{
    return _nonce;
}


void AccountState::incNonce() noexcept
{
    ++_nonce;
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


const base::Sha256& AccountState::getCodeHash() const noexcept
{
    return _code_hash;
}


void AccountState::setCodeHash(base::Sha256 code_hash)
{
    _code_hash = std::move(code_hash);
}


bool AccountState::checkStorageValue(const base::Sha256& key) const
{
    return _storage.find(key) != _storage.end();
}


AccountState::StorageData AccountState::getStorageValue(const base::Sha256& key) const
{
    if(auto it = _storage.find(key); it == _storage.end()) {
        RAISE_ERROR(base::LogicError, "value was not found by a given key");
    }
    else {
        return it->second;
    }
}


void AccountState::setStorageValue(const base::Sha256& key, base::Bytes value)
{
    StorageData& sd = _storage[key];
    sd.data = std::move(value);
    sd.was_modified = true;
}


void AccountManager::newAccount(const bc::Address& address, base::Bytes associated_code)
{
    if(hasAccount(address)) {
        RAISE_ERROR(base::LogicError, "address already exists");
    }
    auto code_hash = (associated_code.isEmpty() ? base::Sha256::null() : base::Sha256::compute(associated_code));
    if(auto it = _code_db.find(code_hash); it == _code_db.end()) {
        _code_db[code_hash] = std::move(associated_code);
    }

    AccountState state;
    state.setCodeHash(code_hash);
    _states[address] = std::move(state);
}


bool AccountManager::hasAccount(const bc::Address& address) const
{
    return _states.find(address) != _states.end();
}


bool AccountManager::deleteAccount(const bc::Address& address)
{
    if(auto it = _states.find(address); it != _states.end()) {
        _states.erase(it);
        return true;
    }
    else {
        return false;
    }
}


bc::Address AccountManager::newContract(const bc::Address& address, base::Bytes associated_code)
{
    auto& account = getAccount(address);
    auto nonce = account.getNonce() + 1;
    account.incNonce();
    auto bytes_address = address.getBytes();
    bytes_address[0] = (bytes_address[0] + nonce) & 0xFF; // TEMPORARILY!!
    auto account_address = bc::Address(bytes_address);
    newAccount(account_address, associated_code);
    return account_address;
}


const AccountState& AccountManager::getAccount(const bc::Address& address) const
{
    std::shared_lock lk(_rw_mutex);
    auto it = _states.find(address);
    if(it == _states.end()) {
        RAISE_ERROR(base::InvalidArgument, "cannot getAccount for non-existent account");
    }
    else {
        return it->second;
    }
}


AccountState& AccountManager::getAccount(const bc::Address& address)
{
    std::shared_lock lk(_rw_mutex);
    auto it = _states.find(address);
    if(it == _states.end()) {
        AccountState state; // TODO: lazy creation
        return _states[address] = state;
    }
    else {
        return it->second;
    }
}


bc::Balance AccountManager::getBalance(const bc::Address& account_address) const
{
    if(hasAccount(account_address)) {
        return getAccount(account_address).getBalance();
    }
    else {
        return 0;
    }
}


const base::Bytes& AccountManager::getCode(const base::Sha256& hash) const
{
    if(auto it = _code_db.find(hash); it == _code_db.end()) {
        RAISE_ERROR(base::LogicError, "cannot find given hash in codes db");
    }
    else {
        return it->second;
    }
}


bool AccountManager::checkTransaction(const bc::Transaction& tx) const
{
    std::shared_lock lk(_rw_mutex);
    if(_states.find(tx.getFrom()) == _states.end()) {
        return false;
    }
    return getAccount(tx.getFrom()).getBalance() >= tx.getAmount();
}


bool AccountManager::tryTransferMoney(const bc::Address& from, const bc::Address& to, bc::Balance amount)
{
    if(!hasAccount(from)) {
        return false;
    }
    auto& from_account = getAccount(from);
    if(from_account.getBalance() < amount) {
        return false;
    }
    if(!hasAccount(to)) {
        newAccount(to, {});
    }
    auto& to_account = getAccount(to);

    from_account.subBalance(amount);
    to_account.addBalance(amount);
    return true;
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
    from_state.incNonce();
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


std::optional<std::reference_wrapper<const base::Bytes>> CodeManager::getCode(const base::Sha256& hash) const
{
    if(auto it = _code_db.find(hash); it == _code_db.end()) {
        return std::nullopt;
    }
    else {
        return it->second;
    }
}


void CodeManager::saveCode(base::Bytes code)
{
    auto hash = base::Sha256::compute(code);
    _code_db.insert({std::move(hash), std::move(code)});
}

} // namespace lk
