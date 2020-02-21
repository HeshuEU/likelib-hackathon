#pragma once

#include "bc/block.hpp"
#include "bc/transaction.hpp"

#include <map>
#include <shared_mutex>


namespace lk
{

class AccountManager
{
  public:
    //================
    explicit AccountManager() = default;
    explicit AccountManager(const std::map<bc::Address, bc::Balance>& initial_state);
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
    std::map<bc::Address, bc::Balance> _storage;
    mutable std::shared_mutex _rw_mutex;
    //================
};

} // namespace lk
