#include "managers.hpp"

#include "base/error.hpp"

namespace lk
{

AccountState::AccountState()
  : _type{ AccountType::CLIENT }
{}


AccountState::AccountState(AccountType type)
  : _type{ type }
{}


AccountType AccountState::getType() const
{
    return _type;
}

std::uint64_t AccountState::getNonce() const noexcept
{
    return _nonce;
}


void AccountState::addTransactionHash(base::Sha256 tx_hash)
{
    _transactions.emplace_back(std::move(tx_hash));
    ++_nonce;
}
//
// void AccountState::incNonce() noexcept {}


lk::Balance AccountState::getBalance() const noexcept
{
    return _balance;
}


void AccountState::setBalance(lk::Balance new_balance)
{
    _balance = new_balance;
}


void AccountState::addBalance(lk::Balance delta)
{
    _balance += delta;
}


void AccountState::subBalance(lk::Balance delta)
{
    if (_balance < delta) {
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


void AccountState::setAbi(const base::PropertyTree& abi)
{
    _abi = abi;
}


const base::PropertyTree& AccountState::getAbi() const
{
    return _abi;
}


void AccountState::setRuntimeCode(const base::Bytes& code)
{
    _runtime_code = code;
}


const base::Bytes& AccountState::getRuntimeCode() const
{
    return _runtime_code;
}


bool AccountState::checkStorageValue(const base::Sha256& key) const
{
    return _storage.find(key) != _storage.end();
}


AccountState::StorageData AccountState::getStorageValue(const base::Sha256& key) const
{
    if (auto it = _storage.find(key); it == _storage.end()) {
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


AccountInfo AccountState::toInfo() const
{
    if (_code_hash == base::Sha256::null()) {
        AccountInfo info{ AccountType::CLIENT, lk::Address::null(), _balance, _nonce, _transactions, "" };
        return info;
    }
    else {
        auto serialized_abi = _abi.toString();
        AccountInfo info{ AccountType::CONTRACT, lk::Address::null(), _balance, _nonce, _transactions, serialized_abi };
        return info;
    }
}


void StateManager::createClientAccount(const lk::Address& address)
{
    if (hasAccount(address)) {
        RAISE_ERROR(base::LogicError, "address already exists");
    }

    std::unique_lock lk(_rw_mutex);
    AccountState state{ AccountType::CLIENT };
    _states.insert({ address, state });
}


lk::Address StateManager::createContractAccount(const lk::Address& from_account_address,
                                                base::Sha256 associated_code_hash)
{
    // address = Ripemd160(associated_code_hash + account_address + Bytes(to_string(nonce))
    auto& account = getAccount(from_account_address);
    //    auto nonce = account.getNonce() + 1;
    //    account.addTransactionHash(tx.hashOfTransaction());
    base::Bytes nonce_string_data{ std::to_string(account.getNonce()) };

    auto bytes_address = base::Ripemd160::compute(associated_code_hash.getBytes().toBytes() +
                                                  from_account_address.getBytes().toBytes() + nonce_string_data);
    auto account_address = lk::Address(bytes_address.getBytes());

    AccountState state{ AccountType::CONTRACT };
    state.setCodeHash(associated_code_hash);
    _states[account_address] = std::move(state);
    return account_address;
}

// void StateManager::newAccount(const lk::Address& address, base::Sha256 code_hash)
//{
//    if (hasAccount(address)) {
//        RAISE_ERROR(base::LogicError, "address already exists");
//    }
//
//    std::unique_lock lk(_rw_mutex);
//    AccountState state;
//    state.setCodeHash(std::move(code_hash));
//    _states[address] = std::move(state);
//}


bool StateManager::hasAccount(const lk::Address& address) const
{
    std::shared_lock lk(_rw_mutex);
    return _states.find(address) != _states.end();
}


bool StateManager::deleteAccount(const lk::Address& address)
{
    std::unique_lock lk(_rw_mutex);
    if (auto it = _states.find(address); it != _states.end()) {
        _states.erase(it);
        return true;
    }
    else {
        return false;
    }
}


// lk::Address StateManager::newContract(const lk::Address& address, base::Sha256 associated_code_hash)
//{
//    // address = Ripemd160(associated_code_hash + account_address + Bytes(to_string(nonce))
//    auto& account = getAccount(address);
//    auto nonce = account.getNonce() + 1;
//    account.incNonce();
//    base::Bytes nonce_string_data{ std::to_string(nonce) };
//
//    auto bytes_address = base::Ripemd160::compute(associated_code_hash.getBytes().toBytes() +
//                                                  address.getBytes().toBytes() + nonce_string_data);
//    auto account_address = lk::Address(bytes_address.getBytes());
//
//    newAccount(account_address, std::move(associated_code_hash));
//    return account_address;
//}


const AccountState& StateManager::getAccount(const lk::Address& address) const
{
    std::shared_lock lk(_rw_mutex);
    auto it = _states.find(address);
    if (it == _states.end()) {
        RAISE_ERROR(base::InvalidArgument, "cannot getAccount for non-existent account");
    }
    else {
        return it->second;
    }
}


AccountState& StateManager::getAccount(const lk::Address& address)
{
    std::shared_lock lk(_rw_mutex);
    auto it = _states.find(address);
    if (it == _states.end()) {
        AccountState state(AccountType::CLIENT); // TODO: lazy creation
        return _states[address] = state;
    }
    else {
        return it->second;
    }
}


lk::Balance StateManager::getBalance(const lk::Address& account_address) const
{
    if (hasAccount(account_address)) {
        return getAccount(account_address).getBalance();
    }
    else {
        return 0;
    }
}


TransactionStatus StateManager::getTransactionOutput(const base::Sha256& tx)
{
    std::shared_lock lk(_tx_outputs_mutex);
    if (auto it = _tx_outputs.find(tx); it != _tx_outputs.end()) {
        return it->second;
    }
    else {
        return TransactionStatus{ TransactionStatus::StatusCode::Failed, TransactionStatus::ActionType::None, 0, "" };
    }
}


void StateManager::addTransactionOutput(const base::Sha256& tx, const TransactionStatus& status)
{
    _tx_outputs.insert({ tx, status });
}


bool StateManager::checkTransaction(const lk::Transaction& tx) const
{
    std::shared_lock lk(_rw_mutex);
    if (_states.find(tx.getFrom()) == _states.end()) {
        return false;
    }
    return getAccount(tx.getFrom()).getBalance() >= tx.getAmount();
}


bool StateManager::tryTransferMoney(const lk::Address& from, const lk::Address& to, lk::Balance amount)
{
    if (!hasAccount(from)) {
        return false;
    }
    auto& from_account = getAccount(from);
    if (from_account.getBalance() < amount) {
        return false;
    }
    if (!hasAccount(to)) {
        createClientAccount(to);
    }
    auto& to_account = getAccount(to);

    from_account.subBalance(amount);
    to_account.addBalance(amount);
    return true;
}


// void StateManager::update(const lk::Transaction& tx)
//{
//    std::unique_lock lk(_rw_mutex);
//    auto from_iter = _states.find(tx.getFrom());
//
//    if (from_iter == _states.end() || from_iter->second.getBalance() < tx.getAmount()) {
//        RAISE_ERROR(base::LogicError, "account doesn't have enough funds to perform the operation");
//    }
//
//    auto& from_state = from_iter->second;
//    from_state.subBalance(tx.getAmount());
//    if (auto to_iter = _states.find(tx.getTo()); to_iter != _states.end()) {
//        to_iter->second.addBalance(tx.getAmount());
//    }
//    else {
//        AccountState to_state;
//        to_state.setBalance(tx.getAmount());
//        _states.insert({ tx.getTo(), std::move(to_state) });
//    }
//    from_state.incNonce();
//}
//
//
// void StateManager::update(const lk::Block& block)
//{
//    for (const auto& tx : block.getTransactions()) {
//        update(tx);
//    }
//}


void StateManager::updateFromGenesis(const lk::Block& block)
{
    std::unique_lock lk(_rw_mutex);
    for (const auto& tx : block.getTransactions()) {
        AccountState state{ AccountType::CLIENT };
        state.setBalance(tx.getAmount());
        _states.insert({ tx.getTo(), std::move(state) });
    }
}

//
// std::optional<std::reference_wrapper<const base::Bytes>> CodeManager::getCode(const base::Sha256& hash) const
//{
//    if (auto it = _code_db.find(hash); it == _code_db.end()) {
//        return std::nullopt;
//    }
//    else {
//        return it->second;
//    }
//}
//
//
// void CodeManager::saveCode(base::Bytes code)
//{
//    auto hash = base::Sha256::compute(code);
//    _code_db.insert({ std::move(hash), std::move(code) });
//}

} // namespace core
