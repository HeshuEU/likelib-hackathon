#pragma once

#include "bc/block.hpp"
#include "bc/transaction.hpp"

#include <map>
#include <shared_mutex>


namespace lk
{

class BalanceManager
{
  public:
    //================
    explicit BalanceManager() = default;
    explicit BalanceManager(const std::map<bc::Address, bc::Balance>& initial_state);
    BalanceManager(const BalanceManager& hash) = delete;
    BalanceManager(BalanceManager&& hash) = default;

    BalanceManager& operator=(const BalanceManager& another) = delete;
    BalanceManager& operator=(BalanceManager&& another) = delete;
    ~BalanceManager() = default;
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
