#include "managers.hpp"

#include "base/error.hpp"

namespace lk
{

AccountState::AccountState(AccountType initial_type)
  : type{ initial_type }
  , nonce{ 0 }
  , code_hash{ base::Sha256::null() }
{}


Commit::Commit(StateManager& state_manager)
  : _state_manager{ state_manager }
{}

Commit::Commit(Commit&& another)
  : _state_manager{ another._state_manager }
{
    _changed_states = std::move(another._changed_states);
    _deleted_accounts = std::move(another._deleted_accounts);
}


Commit& Commit::operator=(Commit&& another)
{
    std::scoped_lock lock{ _rw_mutex, another._rw_mutex };
    _changed_states = std::move(another._changed_states);
    _deleted_accounts = std::move(another._deleted_accounts);
    return *this;
}


bool Commit::createClientAccount(const lk::Address& address)
{
    std::unique_lock lock{ _rw_mutex };
    return _createClientAccount(address);
}


lk::Address Commit::createContractAccount(const lk::Address& from_account_address, base::Sha256 associated_code_hash)
{
    std::unique_lock lock{ _rw_mutex };
    if (!_hasAccountAnywhere(from_account_address)) {
        RAISE_ERROR(base::LogicError, "address already exists");
    }


    auto& account = _getAccountAnywhere(from_account_address);
    base::Bytes nonce_string_data{ std::to_string(account.nonce) };

    auto bytes_address = base::Ripemd160::compute(associated_code_hash.getBytes().toBytes() +
                                                  from_account_address.getBytes().toBytes() + nonce_string_data);
    auto account_address = lk::Address(bytes_address.getBytes());

    AccountState state{ AccountType::CONTRACT };
    state.code_hash = std::move(associated_code_hash);
    _changed_states.insert({ account_address, std::move(state) });

    return account_address;
}


bool Commit::hasAccount(const lk::Address& address) const
{
    std::shared_lock lock{ _rw_mutex };
    return _hasAccountAnywhere(address);
}


bool Commit::deleteAccount(const lk::Address& address, const lk::Address& beneficiary_address)
{
    std::unique_lock lock{ _rw_mutex };
    if (!_hasAccountAnywhere(address)) {
        return false;
    }
    auto value = _getAccountAnywhere(address).balance;
    if (!tryTransferMoney(address, beneficiary_address, value)) {
        return false;
    }

    {
        std::unique_lock lock{ _rw_mutex };
        _deleted_accounts.insert(address);
    }

    return false;
}


AccountType Commit::getAccountType(const lk::Address& account_address) const
{
    std::shared_lock lock{ _rw_mutex };
    return _getAccountAnywhere(account_address).type;
}


bool Commit::tryTransferMoney(const lk::Address& from, const lk::Address& to, const lk::Balance& amount)
{
    std::unique_lock lock{ _rw_mutex };
    if (!_hasAccountAnywhere(from)) {
        return false;
    }
    _copyLocalIfNotExists(from);

    if (!_hasAccountAnywhere(to)) {
        ASSERT(_createClientAccount(to));
    }
    else {
        _copyLocalIfNotExists(to);
    }

    auto& from_account = _getAccount(from);
    if (from_account.balance < amount) {
        return false;
    }
    auto& to_account = _getAccount(to);

    from_account.balance -= amount;
    to_account.balance += amount;
    return true;
}


bool Commit::checkStorageValue(const lk::Address& contract_address, const base::Sha256& key) const
{
    std::shared_lock lock{ _rw_mutex };
    if (!_hasAccountAnywhere(contract_address)) {
        RAISE_ERROR(base::LogicError, "account address was not found by a given key");
    }

    const auto& account = _getAccountAnywhere(contract_address);
    if (account.type != AccountType::CONTRACT) {
        RAISE_ERROR(base::LogicError, "account is not a contract type");
    }

    return account.storage.contains(key);
}


const StorageData& Commit::getStorageValue(const lk::Address& contract_address, const base::Sha256& key) const
{
    std::shared_lock lock{ _rw_mutex };
    if (!_hasAccountAnywhere(contract_address)) {
        RAISE_ERROR(base::LogicError, "account address was not found by a given key");
    }

    const auto& account = _getAccountAnywhere(contract_address);
    if (account.type != AccountType::CONTRACT) {
        RAISE_ERROR(base::LogicError, "account is not a contract type");
    }

    if (auto it = account.storage.find(key); it == account.storage.end()) {
        RAISE_ERROR(base::LogicError, "value was not found by a given key");
    }
    else {
        return it->second;
    }
}


void Commit::setStorageValue(const lk::Address& contract_address, const base::Sha256& key, base::Bytes value)
{
    std::shared_lock lock{ _rw_mutex };
    if (!_copyLocalIfNotExists(contract_address)) {
        RAISE_ERROR(base::LogicError, "account address was not found by a given key");
    }
    auto& account = _getAccount(contract_address);
    if (account.type != AccountType::CONTRACT) {
        RAISE_ERROR(base::LogicError, "account is not a contract type");
    }

    StorageData& sd = account.storage[key];
    sd.data = std::move(value);
    sd.was_modified = true;
}


lk::Balance Commit::getBalance(const lk::Address& account_address) const
{
    std::shared_lock lock{ _rw_mutex };
    return _getAccountAnywhere(account_address).balance;
}


std::size_t Commit::getCodeSize(const lk::Address& account_address) const
{
    std::shared_lock lock{ _rw_mutex };
    return _getAccountAnywhere(account_address).runtime_code.size();
}


const base::Sha256& Commit::getCodeHash(const lk::Address& account_address) const
{
    std::shared_lock lock{ _rw_mutex };
    return _getAccountAnywhere(account_address).code_hash;
}


const base::Bytes& Commit::getRuntimeCode(const lk::Address& account_address) const
{
    std::shared_lock lock{ _rw_mutex };
    return _getAccountAnywhere(account_address).runtime_code;
}


void Commit::setRuntimeCode(const lk::Address& contract_address, const base::Bytes& code)
{
    std::shared_lock lock{ _rw_mutex };
    if (!_copyLocalIfNotExists(contract_address)) {
        RAISE_ERROR(base::LogicError, "account address was not found by a given key");
    }
    auto& account = _getAccount(contract_address);
    if (account.type != AccountType::CONTRACT) {
        RAISE_ERROR(base::LogicError, "account is not a contract type");
    }

    account.runtime_code = code;
}


AccountState& Commit::_getAccount(const lk::Address& account_address)
{
    auto it = _changed_states.find(account_address);
    if (it == _changed_states.end()) {
        RAISE_ERROR(base::InvalidArgument, "cannot getAccount for non-existent account");
    }
    else {
        return it->second;
    }
}


const AccountState& Commit::_getAccount(const lk::Address& account_address) const
{
    auto it = _changed_states.find(account_address);
    if (it == _changed_states.end() || _deleted_accounts.contains(account_address)) {
        RAISE_ERROR(base::InvalidArgument, "cannot getAccount for non-existent account");
    }
    else {
        return it->second;
    }
}


const AccountState& Commit::_getAccountAnywhere(const lk::Address& account_address) const
{
    auto it = _changed_states.find(account_address);
    if (it == _changed_states.end() || _deleted_accounts.contains(account_address)) {
        return _state_manager._getAccount(account_address);
    }
    else {
        return it->second;
    }
}


bool Commit::_hasAccountThis(const lk::Address& address) const
{
    return _changed_states.contains(address) && !_deleted_accounts.contains(address);
}


bool Commit::_hasAccountRoot(const lk::Address& address) const
{
    return _state_manager.hasAccount(address);
}


bool Commit::_hasAccountAnywhere(const lk::Address& address) const
{
    return _hasAccountThis(address) || _hasAccountRoot(address);
}


bool Commit::_copyLocalIfNotExists(const lk::Address& address)
{
    if (!_hasAccountAnywhere(address)) {
        return false;
    }
    if (!_hasAccountThis(address)) {
        auto& account = _state_manager._getAccount(address);
        _changed_states.insert({ address, account });
    }
    return true;
}


bool Commit::_createClientAccount(const lk::Address& address)
{
    if (_hasAccountAnywhere(address)) {
        return false;
    }

    AccountState state{ AccountType::CLIENT };
    _changed_states.insert({ address, state });
    return true;
}


bool StateManager::checkTransaction(const lk::Transaction& tx) const
{
    std::shared_lock lk(_rw_mutex);
    if (!_hasAccount(tx.getFrom())) {
        return false;
    }
    return _getAccount(tx.getFrom()).balance >= tx.getAmount() + tx.getFee();
}


bool StateManager::checkTransactionsSet(const lk::TransactionsSet& tx_set) const
{
    std::shared_lock lk(_rw_mutex);
    auto block_balance = lk::calcCost(tx_set);
    for (const auto& tx : tx_set) {
        if (_hasAccount(tx.getFrom())) {
            auto current_account_balance = _getBalance(tx.getFrom());
            auto block_account_cost = block_balance.find(tx.getFrom());
            ASSERT(block_account_cost != block_balance.end());
            if (block_account_cost->second > current_account_balance) {
                return false;
            }
        }
        else {
            return false;
        }
    }
    return true;
}


void StateManager::updateFromGenesis(const ImmutableBlock& block)
{
    std::unique_lock lk(_rw_mutex);
    for (const auto& tx : block.getTransactions()) {
        AccountState state{ AccountType::CLIENT };
        state.balance = tx.getAmount();
        _states.insert({ tx.getTo(), std::move(state) });
    }
}


Commit StateManager::createCommit()
{
    return Commit{ *this };
}


void StateManager::applyCommit(Commit&& commit)
{
    std::set<lk::Address> updated_set;
    {
        std::unique_lock lk(_rw_mutex);
        for (auto& changed_account : commit._changed_states) {
            auto it = _states.find(changed_account.first);
            if (it == _states.end()) {
                _states.insert(changed_account);
            }
            else {
                it->second = changed_account.second;
            }

            updated_set.insert(changed_account.first);
        }
        for (auto& deleted_account_address : commit._deleted_accounts) {
            _states.erase(deleted_account_address);
            updated_set.insert(deleted_account_address);
        }
    }

    for (auto& updated_account : updated_set) {
        _event_account_update.notify(updated_account);
    }
}


void StateManager::addTxHash(const lk::Address& address, const base::Sha256& tx_hash)
{
    std::shared_lock lk(_rw_mutex);
    if (!_hasAccount(address)) {
        ASSERT(_createClientAccount(address));
    }
    auto& account = _getAccount(address);
    ASSERT(account.type == AccountType::CLIENT);

    account.transactions.emplace_back(std::move(tx_hash));
    ++(account.nonce);
}


void StateManager::applyBlockEmission(const lk::Address& address, const lk::Balance& value)
{
    std::unique_lock lk(_rw_mutex);
    if (!_hasAccount(address)) {
        ASSERT(_createClientAccount(address));
    }
    auto& account = _getAccount(address);
    account.balance += value;

    _event_account_update.notify(address);
}


bool StateManager::payFee(const lk::Address& from, const lk::Address& to, const lk::Balance& value)
{
    std::unique_lock lk(_rw_mutex);
    if (!_hasAccount(from)) {
        return false;
    }
    auto& from_account = _getAccount(from);
    if (from_account.balance < value) {
        return false;
    }
    if (!_hasAccount(to)) {
        ASSERT(_createClientAccount(to));
    }
    auto& to_account = _getAccount(to);

    from_account.balance -= value;
    to_account.balance += value;

    _event_account_update.notify(from);
    _event_account_update.notify(to);

    return true;
}


bool StateManager::hasAccount(const lk::Address& address) const
{
    std::shared_lock lk(_rw_mutex);
    return _hasAccount(address);
}


AccountInfo StateManager::getAccountInfo(const lk::Address& account_address) const
{
    std::shared_lock lk(_rw_mutex);
    if (!_hasAccount(account_address)) {
        return AccountInfo{ AccountType::CLIENT, lk::Address::null(), {}, {}, {} };
    }
    const auto& account = _getAccount(account_address);
    return AccountInfo{ account.type, account_address, account.balance, account.nonce, account.transactions };
}


lk::Balance StateManager::getBalance(const lk::Address& account_address) const
{
    std::shared_lock lk(_rw_mutex);
    return _getBalance(account_address);
}


AccountState& StateManager::_getAccount(const lk::Address& account_address)
{
    auto it = _states.find(account_address);
    if (it == _states.end()) {
        RAISE_ERROR(base::InvalidArgument, "cannot getAccount for non-existent account");
    }
    else {
        return it->second;
    }
}


const AccountState& StateManager::_getAccount(const lk::Address& account_address) const
{
    auto it = _states.find(account_address);
    if (it == _states.end()) {
        RAISE_ERROR(base::InvalidArgument, "cannot getAccount for non-existent account");
    }
    else {
        return it->second;
    }
}


bool StateManager::_hasAccount(const lk::Address& address) const
{
    return _states.contains(address);
}


bool StateManager::_createClientAccount(const lk::Address& address)
{
    if (_hasAccount(address)) {
        return false;
    }

    AccountState state{ AccountType::CLIENT };
    _states.insert({ address, state });
    return true;
}

lk::Balance StateManager::_getBalance(const lk::Address& account_address) const
{
    if (_hasAccount(account_address)) {
        return _getAccount(account_address).balance;
    }
    return {};
}

std::size_t StateManager::subscribeToAnyAccountUpdate(decltype(_event_account_update)::CallbackType callback)
{
    return _event_account_update.subscribe(std::move(callback));
}


} // namespace core
