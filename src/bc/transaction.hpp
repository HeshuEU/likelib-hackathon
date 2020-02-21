#pragma once

#include "base/crypto.hpp"
#include "base/serialization.hpp"
#include "base/time.hpp"

#include "bc/address.hpp"
#include "bc/types.hpp"

namespace bc
{

class Sign
{
  public:
    Sign() = default;
    Sign(base::RsaPublicKey sender_public_key, base::Bytes rsa_encrypted_hash);

    [[nodiscard]] bool isNull() const noexcept;

    [[nodiscard]] const base::RsaPublicKey& getPublicKey() const;
    [[nodiscard]] const base::Bytes& getRsaEncryptedHash() const;

    void serialize(base::SerializationOArchive& oa) const;
    static Sign deserialize(base::SerializationIArchive& ia);

  private:
    struct Data
    {
        base::RsaPublicKey sender_public_key;
        base::Bytes rsa_encrypted_hash;
    };

    std::optional<Data> _data;
};


class Transaction
{
  public:
    //=================
    Transaction(bc::Address from, bc::Address to, bc::Balance amount, bc::Balance fee, base::Time timestamp, bc::Sign sign = bc::Sign{});
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
    [[nodiscard]] const bc::Balance& getFee() const noexcept;
    //=================
    void sign(base::RsaPublicKey pub, const base::RsaPrivateKey& priv);
    [[nodiscard]] bool checkSign() const;
    [[nodiscard]] const bc::Sign& getSign() const noexcept;
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
    bc::Balance _fee;
    base::Time _timestamp;
    bc::Sign _sign;
    //=================
    void serializeHeader(base::SerializationOArchive& oa) const;
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
    void setFee(bc::Balance fee);

    [[nodiscard]] Transaction build() const&;
    [[nodiscard]] Transaction build() &&;

  private:
    std::optional<bc::Address> _from;
    std::optional<bc::Address> _to;
    std::optional<bc::Balance> _amount;
    std::optional<base::Time> _timestamp;
    std::optional<bc::Balance> _fee;
};

} // namespace bc
