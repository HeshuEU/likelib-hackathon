#pragma once

#include "base/crypto.hpp"
#include "base/serialization.hpp"
#include "base/time.hpp"

#include "address.hpp"
#include "types.hpp"

namespace lk
{

class Sign
{
  public:
    Sign() = default;
    Sign(base::RsaPublicKey sender_public_key, base::Bytes rsa_encrypted_hash);

    bool isNull() const noexcept;

    const base::RsaPublicKey& getPublicKey() const;
    const base::Bytes& getRsaEncryptedHash() const;

    static Sign fromBase64(const std::string& base64_signature);
    std::string toBase64() const;

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
    enum class Type : std::uint8_t
    {
        MESSAGE_CALL,
        CONTRACT_CREATION,
    };
    //=================
    Transaction(lk::Address from,
                lk::Address to,
                lk::Balance amount,
                lk::Balance fee,
                base::Time timestamp,
                Type transaction_type,
                base::Bytes data,
                lk::Sign sign = lk::Sign{});
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
    Type getType() const noexcept;
    const base::Bytes& getData() const noexcept;
    const lk::Balance& getFee() const noexcept;
    //=================
    void sign(base::RsaPublicKey pub, const base::RsaPrivateKey& priv);
    bool checkSign() const;
    const lk::Sign& getSign() const noexcept;
    //=================
    bool operator==(const Transaction& other) const;
    bool operator!=(const Transaction& other) const;

    //=================
    static Transaction deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=================
  private:
    //=================
    lk::Address _from;
    lk::Address _to;
    lk::Balance _amount;
    lk::Balance _fee;
    base::Time _timestamp;
    Type _tx_type;
    base::Bytes _data;
    lk::Sign _sign;
    //=================
    void serializeHeader(base::SerializationOArchive& oa) const;
    base::Sha256 hashOfTxData() const;
    //=================
};


std::ostream& operator<<(std::ostream& os, const Transaction& tx);


class ContractInitData
{
  public:
    ContractInitData(base::Bytes code, base::Bytes init);

    void setCode(base::Bytes code);
    void setInit(base::Bytes init);

    const base::Bytes& getCode() const noexcept;
    const base::Bytes& getInit() const noexcept;

    void serialize(base::SerializationOArchive& oa) const;
    static ContractInitData deserialize(base::SerializationIArchive& ia);

  private:
    base::Bytes _code;
    base::Bytes _init;
};


class TransactionBuilder
{
  public:
    void setFrom(lk::Address from);
    void setTo(lk::Address to);
    void setAmount(lk::Balance amount);
    void setTimestamp(base::Time timestamp);
    void setFee(lk::Balance fee);
    void setType(Transaction::Type type);
    void setData(base::Bytes data);
    void setSign(lk::Sign sign);

    [[nodiscard]] Transaction build() const&;
    [[nodiscard]] Transaction build() &&;

  private:
    std::optional<lk::Address> _from;
    std::optional<lk::Address> _to;
    std::optional<lk::Balance> _amount;
    std::optional<base::Time> _timestamp;
    std::optional<lk::Balance> _fee;
    std::optional<Transaction::Type> _tx_type;
    std::optional<base::Bytes> _data;
    std::optional<lk::Sign> _sign;
};


const Transaction& invalidTransaction();

} // namespace lk
