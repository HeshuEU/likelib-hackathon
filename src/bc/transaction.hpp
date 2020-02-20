#pragma once

#include "bc/address.hpp"
#include "bc/types.hpp"

#include "base/serialization.hpp"
#include "base/time.hpp"

namespace bc
{

class Transaction
{
  public:
    //=================
    Transaction() = default;
    Transaction(const bc::Address& from, const bc::Address& to, const bc::Balance& amount, const base::Time& timestamp);
    Transaction(const Transaction&) = default;
    Transaction(Transaction&&) = default;

    Transaction& operator=(const Transaction&) = default;
    Transaction& operator=(Transaction&&) = default;

    ~Transaction() = default;
    //=================
    const bc::Address& getFrom() const noexcept;
    const bc::Address& getTo() const noexcept;
    const bc::Balance& getAmount() const noexcept;
    const base::Time& getTimestamp() const noexcept;
    //=================
    void setFrom(const bc::Address& from);
    void setTo(const bc::Address& to);
    void setAmount(const bc::Balance& amount);
    void setTimestamp(const base::Time& timestamp);
    //=================
    bool operator==(const Transaction& other) const;
    bool operator!=(const Transaction& other) const;
    base::SerializationOArchive& serialize(base::SerializationOArchive& oa) const;
    static Transaction deserialize(base::SerializationIArchive& ia);
    //=================
  private:
    bc::Address _from;
    bc::Address _to;
    bc::Balance _amount = 1; // TODO: change it later, temporary fix
    base::Time _timestamp;
};

std::ostream& operator<<(std::ostream& os, const Transaction& tx);

} // namespace bc
