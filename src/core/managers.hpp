#pragma once

#include "core/block.hpp"
#include "core/transaction.hpp"

#include "base/utility.hpp"

#include <map>
#include <shared_mutex>

namespace lk
{

enum class AccountType : uint8_t
{
    CLIENT = 0,
    CONTRACT = 1
};


struct AccountInfo
{
    AccountType type;
    lk::Address address;
    lk::Balance balance;
    std::uint64_t nonce;
    std::vector<base::Sha256> transactions_hashes;
};


struct StorageData
{
    StorageData() = default;

    base::Bytes data;
    bool was_modified{ false };
};


struct AccountState
{
    AccountType type;
    std::uint64_t nonce;
    lk::Balance balance;
    base::Sha256 code_hash;
    std::vector<base::Sha256> transactions;
    std::map<base::Sha256, StorageData> storage;
    base::Bytes runtime_code;
    //============================
    explicit AccountState(AccountType initial_type);
    ~AccountState() = default;
};


class StateManager;


class Commit
{
    friend StateManager;

  public:
    Commit(StateManager& state_manager);
    Commit(Commit&& another);
    Commit& operator=(Commit&& another);
    ~Commit() = default;

    bool createClientAccount(const lk::Address& address);
    lk::Address createContractAccount(const lk::Address& from_account_address, base::Sha256 associated_code_hash);
    bool hasAccount(const lk::Address& address) const;
    bool deleteAccount(const lk::Address& address, const lk::Address& beneficiary_address);
    //================
    AccountType getAccountType(const lk::Address& account_address) const;
    //================
    bool tryTransferMoney(const lk::Address& from, const lk::Address& to, const lk::Balance& amount);
    //================
    bool checkStorageValue(const lk::Address& contract_address, const base::Sha256& key) const;
    const StorageData& getStorageValue(const lk::Address& contract_address, const base::Sha256& key) const;
    void setStorageValue(const lk::Address& contract_address, const base::Sha256& key, base::Bytes value);
    lk::Balance getBalance(const lk::Address& account_address) const;
    std::size_t getCodeSize(const lk::Address& account_address) const;
    const base::Sha256& getCodeHash(const lk::Address& account_address) const;
    const base::Bytes& getRuntimeCode(const lk::Address& account_address) const;
    void setRuntimeCode(const lk::Address& contract_address, const base::Bytes& code);

  private:
    StateManager& _state_manager;
    std::map<lk::Address, AccountState> _changed_states;
    std::set<lk::Address> _deleted_accounts;
    mutable std::shared_mutex _rw_mutex;

    AccountState& _getAccount(const lk::Address& account_address);
    const AccountState& _getAccount(const lk::Address& account_address) const;
    const AccountState& _getAccountAnywhere(const lk::Address& account_address) const;
    bool _hasAccountThis(const lk::Address& address) const;
    bool _hasAccountRoot(const lk::Address& address) const;
    bool _hasAccountAnywhere(const lk::Address& address) const;
    bool _copyLocalIfNotExists(const lk::Address& address);
    bool _createClientAccount(const lk::Address& address);
};


class StateManager
{
    friend Commit;

  public:
    //================
    StateManager() = default;
    StateManager(const StateManager&) = delete;
    StateManager(StateManager&& other) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager& operator=(StateManager&&) = delete;
    ~StateManager() = default;
    //================
    bool checkTransaction(const lk::Transaction& tx) const;
    bool checkTransactionsSet(const lk::TransactionsSet& tx) const;
    void updateFromGenesis(const ImmutableBlock& block);
    //================
    Commit createCommit();
    void applyCommit(Commit&& commit);
    //================
    void addTxHash(const lk::Address& address, const base::Sha256& tx_hash);
    void applyBlockEmission(const lk::Address& address, const lk::Balance& value);
    bool payFee(const lk::Address& from, const lk::Address& to, const lk::Balance& value);
    //================
    bool hasAccount(const lk::Address& address) const;
    AccountInfo getAccountInfo(const lk::Address& account_address) const;
    lk::Balance getBalance(const lk::Address& account_address) const;

  private:
    //================
    std::map<lk::Address, AccountState> _states;
    mutable std::shared_mutex _rw_mutex;
    //================
    base::Observable<lk::Address> _event_account_update;

    AccountState& _getAccount(const lk::Address& account_address);
    const AccountState& _getAccount(const lk::Address& account_address) const;
    bool _hasAccount(const lk::Address& address) const;
    bool _createClientAccount(const lk::Address& address);
    lk::Balance _getBalance(const lk::Address& account_address) const;

  public:
    std::size_t subscribeToAnyAccountUpdate(decltype(_event_account_update)::CallbackType callback);
};

} // namespace core
