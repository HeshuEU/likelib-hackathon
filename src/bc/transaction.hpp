#pragma once

#include "base/serialization.hpp"
#include "bc/address.hpp"
#include "bc/types.hpp"

namespace bc
{

class Transaction
{
  public:
    //=================
    Transaction() = default;
    Transaction(const bc::Address& from, const bc::Address& to, const bc::Balance& amount);
    Transaction(const Transaction&) = default;
    Transaction(Transaction&&) = default;

    Transaction& operator=(const Transaction&) = default;
    Transaction& operator=(Transaction&&) = default;

    ~Transaction() = default;
    //=================
    const bc::Address& getFrom() const noexcept;
    const bc::Address& getTo() const noexcept;
    const bc::Balance& getAmount() const noexcept;
    //=================
    void setFrom(const bc::Address& from);
    void setTo(const bc::Address& to);
    void setAmount(const bc::Balance& amount);
    //=================
    bool operator==(const Transaction& other) const;
    bool operator!=(const Transaction& other) const;
  private:
    bc::Address _from;
    bc::Address _to;
    bc::Balance _amount;
};

base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Transaction& tx);
base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Transaction& tx);

void removeProcessedTransactions(std::vector<Transaction>& txs, const std::vector<Transaction>& processed);

} // namespace bc