#pragma once

#include "bc/block.hpp"
#include "bc/transaction.hpp"

#include <map>
#include <shared_mutex>


namespace lk
{


class AccountState
{
  public:
    //============================
    struct StorageData
    {
        StorageData() = default;

        base::Bytes data;
        bool was_modified{false};
    };
    //============================
    std::uint64_t getNonce() const noexcept;
    void incNonce() noexcept;
    //============================
    bc::Balance getBalance() const noexcept;
    void setBalance(bc::Balance new_balance);
    void addBalance(bc::Balance delta);
    void subBalance(bc::Balance delta);
    //============================
    const base::Sha256& getCodeHash() const noexcept;
    void setCodeHash(base::Sha256 code_hash);
    //============================
    bool checkStorageValue(const base::Sha256& key) const;
    StorageData getStorageValue(const base::Sha256& key) const;
    void setStorageValue(const base::Sha256& key, base::Bytes value);
    //============================
  private:
    std::uint64_t _nonce;
    bc::Balance _balance;
    base::Sha256 _code_hash{ base::Sha256::null() };
    std::map<base::Sha256, StorageData> _storage;
};


class AccountManager
{
  public:
    //================
    AccountManager() = default;
    AccountManager(const AccountManager& hash) = delete;
    AccountManager(AccountManager&& hash) = delete;

    AccountManager& operator=(const AccountManager& another) = delete;
    AccountManager& operator=(AccountManager&& another) = delete;
    ~AccountManager() = default;
    //================
    void newAccount(const bc::Address& address, base::Bytes associated_code);
    bool hasAccount(const bc::Address& address) const;
    //================
    bool checkTransaction(const bc::Transaction& tx) const;
    void update(const bc::Transaction& tx);
    void update(const bc::Block& block);
    void updateFromGenesis(const bc::Block& block);
    //================
    const AccountState& getAccount(const bc::Address& address) const;
    AccountState& getAccount(const bc::Address& address);
    //================
    const base::Bytes& getCode(const base::Sha256& hash) const;
    //================
  private:
    //================
    std::map<bc::Address, AccountState> _states;
    std::map<base::Sha256, base::Bytes> _code_db;
    mutable std::shared_mutex _rw_mutex;
    //================
};

} // namespace lk
