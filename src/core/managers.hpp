#pragma once

#include "core/block.hpp"
#include "core/transaction.hpp"

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


class AccountState
{
  public:
    //============================
    struct StorageData
    {
        StorageData() = default;

        base::Bytes data;
        bool was_modified{ false };
    };
    //============================
    explicit AccountState();
    explicit AccountState(AccountType type);
    ~AccountState() = default;
    //============================
    AccountType getType() const;
    //============================
    std::uint64_t getNonce() const noexcept;
    void addTransactionHash(base::Sha256 tx_hash);
    //============================
    lk::Balance getBalance() const noexcept;
    void setBalance(lk::Balance new_balance);
    void addBalance(lk::Balance delta);
    void subBalance(lk::Balance delta);
    //============================
    const base::Sha256& getCodeHash() const noexcept;
    void setCodeHash(base::Sha256 code_hash);
    //============================
    void setRuntimeCode(const base::Bytes& code);
    const base::Bytes& getRuntimeCode() const;
    //============================
    bool checkStorageValue(const base::Sha256& key) const;
    StorageData getStorageValue(const base::Sha256& key) const;
    void setStorageValue(const base::Sha256& key, base::Bytes value);
    //============================
    AccountInfo toInfo() const;

  private:
    AccountType _type{ AccountType::CLIENT };
    std::uint64_t _nonce{ 0 };
    lk::Balance _balance{ 0 };
    base::Sha256 _code_hash{ base::Sha256::null() };
    std::vector<base::Sha256> _transactions;
    std::map<base::Sha256, StorageData> _storage;
    base::Bytes _runtime_code;
};


class StateManager
{
  public:
    //================
    StateManager() = default;
    StateManager(const StateManager&) = delete;
    StateManager(StateManager&& other);

    StateManager& operator=(const StateManager&) = delete;
    StateManager& operator=(StateManager&&) noexcept;
    ~StateManager() = default;
    //================
    void createClientAccount(const lk::Address& address);
    lk::Address createContractAccount(const lk::Address& from_account_address, base::Sha256 associated_code_hash);
    bool hasAccount(const lk::Address& address) const;
    bool deleteAccount(const lk::Address& address);
    //================
    bool tryTransferMoney(const lk::Address& from, const lk::Address& to, const lk::Balance& amount);
    //================
    bool checkTransaction(const lk::Transaction& tx) const;
    void updateFromGenesis(const ImmutableBlock& block);
    //================
    const AccountState& getAccount(const lk::Address& account_address) const;
    AccountState& getAccount(const lk::Address& address);
    //================
    StateManager createCopy();
    void applyChanges(StateManager&& state);

  private:
    //================
    std::map<lk::Address, AccountState> _states;
    mutable std::shared_mutex _rw_mutex;
};

} // namespace core
