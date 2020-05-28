#pragma once

#include "core/address.hpp"
#include "core/types.hpp"

#include "base/crypto.hpp"
#include "base/serialization.hpp"
#include "base/time.hpp"

namespace lk
{

using Sign = base::FixedBytes<base::Secp256PrivateKey::SECP256_SIGNATURE_SIZE>;


class Transaction
{
  public:
    Transaction(Address from,
                Address to,
                Balance amount,
                Fee fee,
                base::Time timestamp,
                base::Bytes data,
                Sign sign = lk::Sign{});
    Transaction(const Transaction&) = default;
    Transaction(Transaction&&) = default;

    Transaction& operator=(const Transaction&) = default;
    Transaction& operator=(Transaction&&) = default;

    ~Transaction() = default;
    //=================
    const lk::Address& getFrom() const noexcept;
    const lk::Address& getTo() const noexcept;
    const lk::Balance& getAmount() const noexcept;
    const base::Time& getTimestamp() const noexcept;
    const base::Bytes& getData() const noexcept;
    const std::uint64_t& getFee() const noexcept;
    //=================
    void sign(const base::Secp256PrivateKey& key);
    bool checkSign() const;
    const lk::Sign& getSign() const noexcept;
    //=================
    bool operator==(const Transaction& other) const;
    bool operator!=(const Transaction& other) const;
    //=================
    base::Sha256 hashOfTransaction() const;
    //=================
    static Transaction deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=================
  private:
    //=================
    Address _from;
    Address _to;
    Balance _amount;
    Fee _fee;
    base::Time _timestamp;
    base::Bytes _data;
    Sign _sign;
    //=================
};


std::ostream& operator<<(std::ostream& os, const Transaction& tx);


class TransactionBuilder
{
  public:
    void setFrom(Address from);
    void setTo(Address to);
    void setAmount(Balance amount);
    void setTimestamp(base::Time timestamp);
    void setFee(Fee fee);
    void setData(base::Bytes data);
    void setSign(Sign sign);

    [[nodiscard]] Transaction build() const&;
    [[nodiscard]] Transaction build() &&;

  private:
    std::optional<Address> _from;
    std::optional<Address> _to;
    std::optional<Balance> _amount;
    std::optional<base::Time> _timestamp;
    std::optional<Fee> _fee;
    std::optional<base::Bytes> _data;
    std::optional<Sign> _sign;
};


const Transaction& invalidTransaction();


class TransactionStatus
{
  public:
    enum class StatusCode : uint8_t
    {
        Success = 0,
        Pending = 1,
        BadQueryForm = 2,
        BadSign = 3,
        NotEnoughBalance = 4,
        Revert = 5,
        Failed = 6
    };

    enum class ActionType : uint8_t
    {
        None = 0,
        Transfer = 1,
        ContractCall = 2,
        ContractCreation = 3
    };

    explicit TransactionStatus(StatusCode status,
                               ActionType type,
                               std::uint64_t fee_left,
                               const std::string& message = "") noexcept;

    TransactionStatus(const TransactionStatus&) = default;
    TransactionStatus(TransactionStatus&&) = default;
    TransactionStatus& operator=(const TransactionStatus&) = default;
    TransactionStatus& operator=(TransactionStatus&&) = default;

    operator bool() const noexcept;

    bool operator!() const noexcept;

    const std::string& getMessage() const noexcept;
    std::string& getMessage() noexcept;

    StatusCode getStatus() const noexcept;

    ActionType getType() const noexcept;

    std::uint64_t getFeeLeft() const noexcept;

  private:
    StatusCode _status;
    ActionType _action;
    std::string _message;
    Fee _fee_left;
};

} // namespace lk
