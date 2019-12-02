#pragma once

#include "bc/transaction.hpp"

#include <map>


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
    bool checkTransaction(const Transaction& tx);
    void update(const Transaction& tx);
    Balance getBalance(const Address& address) const;
    //================
  private:
    //================
    std::map<Address, Balance> _storage;
    //================
};

}; // namespace bc
