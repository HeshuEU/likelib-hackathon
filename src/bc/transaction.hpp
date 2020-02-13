#pragma once

#include "base/crypto.hpp"
#include "base/serialization.hpp"
#include "base/time.hpp"

#include "bc/address.hpp"
#include "bc/types.hpp"

namespace bc
{

class Transaction
{
  public:
    //=================
    Transaction(bc::Address from, bc::Address to, bc::Balance amount, base::Time timestamp);
    Transaction(const Transaction&) = default;
    Transaction(Transaction&&) = default;

    Transaction& operator=(const Transaction&) = default;
    Transaction& operator=(Transaction&&) = default;

    ~Transaction() = default;
    //=================
    [[nodiscard]] const bc::Address& getFrom() const noexcept;
    [[nodiscard]] const bc::Address& getTo() const noexcept;
    [[nodiscard]] const bc::Balance& getAmount() const noexcept;
    [[nodiscard]] const base::Time& getTimestamp() const noexcept;
    //=================
    void sign(const base::RsaPrivateKey& priv);
    bool checkSign(const base::RsaPublicKey& pub) const;
    //=================
    bool operator==(const Transaction& other) const;
    bool operator!=(const Transaction& other) const;

    //=================
    static Transaction deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=================
  private:
    //=================
    bc::Address _from;
    bc::Address _to;
    bc::Balance _amount;
    base::Time _timestamp;
    base::Bytes _sign; // unsigned if empty
    //=================
    base::Sha256 hashOfTxData() const;
    //=================
};

std::ostream& operator<<(std::ostream& os, const Transaction& tx);


class TransactionBuilder
{
  public:
    void setFrom(bc::Address from);
    void setTo(bc::Address to);
    void setAmount(bc::Balance amount);
    void setTimestamp(base::Time timestamp);

    [[nodiscard]] Transaction build() const &;
    [[nodiscard]] Transaction build() &&;

  private:
    bc::Address _from;
    bc::Address _to;
    bc::Balance _amount = 1; // TODO: change it later, temporary fix
    base::Time _timestamp;
};

} // namespace bc
