#pragma once

#include "bc/address.hpp"
#include "bc/types.hpp"

namespace bc
{

class Transaction
{
  public:
    //=================
    Transaction(const bc::Address& from, const bc::Address& to, const bc::Balance& amount);
    Transaction(const Transaction&) = delete; // to prevent from having two equal transactions by mistake
    Transaction(Transaction&&) = default;

    Transaction& operator=(const Transaction&) = delete;
    Transaction& operator=(Transaction&&) = default;

    ~Transaction() = default;
    //=================

    const bc::Address& getFrom() const noexcept;
    const bc::Address& getTo() const noexcept;
    const bc::Balance& getAmount() const noexcept;

    //=================

    base::Bytes serialize() const;

  private:
    bc::Address _from;
    bc::Address _to;
    bc::Balance _amount;
};


class TransactionBuilder
{
  public:
    void setFrom(const bc::Address& from);
    void setTo(const bc::Address& to);
    void setAmount(const bc::Balance& amount);

    Transaction build();

  private:
    bc::Address _from;
    bc::Address _to;
    bc::Balance _amount;
};

} // namespace bc