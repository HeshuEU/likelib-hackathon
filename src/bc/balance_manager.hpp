#pragma once

#include "bc/block.hpp"
#include "bc/transaction.hpp"

#include <map>
#include <shared_mutex>


namespace bc
{

class BalanceManager
{
  public:
    //================
    explicit BalanceManager() = default;
    explicit BalanceManager(const std::map<Address, Balance>& initial_state);
    BalanceManager(const BalanceManager& hash) = delete;
    BalanceManager(BalanceManager&& hash) = default;

    BalanceManager& operator=(const BalanceManager& another) = delete;
    BalanceManager& operator=(BalanceManager&& another) = default;
    ~BalanceManager() = default;
    //================
    bool checkTransaction(const Transaction& tx) const;
    void update(const Transaction& tx);
    void update(const Block& block);
    void updateFromGenesis(const Block& block);   //TODO:make any tests for this?
    Balance getBalance(const Address& address) const;
    //================
  private:
    //================
    std::map<Address, Balance> _storage;
    mutable std::shared_mutex _rw_mutex;
    //================
};

} // namespace bc
