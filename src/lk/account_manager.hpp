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
    std::uint64_t getNonce() const noexcept;

    bc::Balance getBalance() const noexcept;
    void setBalance(bc::Balance new_balance);
    void addBalance(bc::Balance delta);
    void subBalance(bc::Balance delta);

    const base::Bytes& getCode() const noexcept;

    bool checkStorageValue(const base::Sha256& key) const;
    std::optional<std::reference_wrapper<const base::Bytes>> getStorageValue(const base::Sha256& key) const;
    void setStorageValue(const base::Sha256& key, base::Bytes value);

  private:
    std::uint64_t _nonce;
    bc::Balance _balance;
    base::Bytes _code;
    std::map<base::Sha256, base::Bytes> _storage;
};


class AccountManager
{
  public:
    //================
    explicit AccountManager() = default;
    AccountManager(const AccountManager& hash) = delete;
    AccountManager(AccountManager&& hash) = delete;

    AccountManager& operator=(const AccountManager& another) = delete;
    AccountManager& operator=(AccountManager&& another) = delete;
    ~AccountManager() = default;
    //================
    bool checkTransaction(const bc::Transaction& tx) const;
    void update(const bc::Transaction& tx);
    void update(const bc::Block& block);
    void updateFromGenesis(const bc::Block& block);
    bc::Balance getBalance(const bc::Address& address) const;
    //================
  private:
    //================
    std::map<bc::Address, AccountState> _states;
    std::map<base::Sha256, base::Bytes> _code_db;
    mutable std::shared_mutex _rw_mutex;
    //================
};

} // namespace lk
