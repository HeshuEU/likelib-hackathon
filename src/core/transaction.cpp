#include "transaction.hpp"

#include "base/error.hpp"


namespace lk
{

Transaction::Transaction(Address from,
                         Address to,
                         Balance amount,
                         Fee fee,
                         base::Time timestamp,
                         base::Bytes data,
                         Sign sign)
  : _from{ std::move(from) }
  , _to{ std::move(to) }
  , _amount{ amount }
  , _fee{ fee }
  , _timestamp{ timestamp }
  , _data{ std::move(data) }
  , _sign{ std::move(sign) }
{
    if ((_amount == 0) && (_fee == 0)) {
        RAISE_ERROR(base::LogicError, "Transaction cannot contain amount equal to 0");
    }
}


const Address& Transaction::getFrom() const noexcept
{
    return _from;
}


const Address& Transaction::getTo() const noexcept
{
    return _to;
}


const Balance& Transaction::getAmount() const noexcept
{
    return _amount;
}


const base::Time& Transaction::getTimestamp() const noexcept
{
    return _timestamp;
}


const Fee& Transaction::getFee() const noexcept
{
    return _fee;
}


const base::Bytes& Transaction::getData() const noexcept
{
    return _data;
}


void Transaction::sign(const base::Secp256PrivateKey& key)
{
    auto hash = hashOfTransaction();
    _sign = key.sign(hash.getBytes().toBytes());
}


bool Transaction::checkSign() const
{
    if (_sign.toBytes().isEmpty()) {
        return false;
    }
    else {
        auto valid_hash = hashOfTransaction();
        try {
            auto pub = base::Secp256PrivateKey::decodeSignatureToPublicKey(_sign, valid_hash.getBytes().toBytes());
            auto derived_addr = lk::Address(pub);
            return _from == derived_addr;
        }
        catch (const base::CryptoError& ex) {
            return false;
        }
    }
}


const Sign& Transaction::getSign() const noexcept
{
    return _sign;
}


bool Transaction::operator==(const Transaction& other) const
{
    return _amount == other._amount && _from == other._from && _to == other._to && _timestamp == other._timestamp &&
           _fee == other._fee && _data == other._data;
}


bool Transaction::operator!=(const Transaction& other) const
{
    return !(*this == other);
}


base::Sha256 Transaction::hashOfTransaction() const
{
    // see public_interface.md
    auto from_address_str = base::base58Encode(_from.getBytes());
    auto to_address_str = base::base58Encode(_to.getBytes());
    auto amount_str = _amount.str();
    auto fee_str = std::to_string(_fee);
    auto timestamp_str = std::to_string(_timestamp.getSeconds());
    auto data_str = base::base64Encode(_data);

    auto concatenated_data = from_address_str + to_address_str + amount_str + fee_str + timestamp_str + data_str;

    return base::Sha256::compute(base::Bytes(concatenated_data));
}


Transaction Transaction::deserialize(base::SerializationIArchive& ia)
{
    auto from = ia.deserialize<lk::Address>();
    auto to = ia.deserialize<lk::Address>();
    auto amount = ia.deserialize<lk::Balance>();
    auto fee = ia.deserialize<std::uint64_t>();
    auto timestamp = ia.deserialize<base::Time>();
    auto data = ia.deserialize<base::Bytes>();
    auto sign = ia.deserialize<lk::Sign>();
    return { std::move(from), std::move(to), amount, fee, timestamp, std::move(data), std::move(sign) };
}


void Transaction::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_from);
    oa.serialize(_to);
    oa.serialize(_amount);
    oa.serialize(_fee);
    oa.serialize(_timestamp);
    oa.serialize(_data);
    oa.serialize(_sign);
}


std::ostream& operator<<(std::ostream& os, const Transaction& tx)
{
    return os << "from: " << tx.getFrom() << " to: " << tx.getTo() << " amount: " << tx.getAmount()
              << " fee: " << tx.getFee() << " timestamp: " << tx.getTimestamp();
}


void TransactionBuilder::setFrom(Address from)
{
    _from = std::move(from);
}


void TransactionBuilder::setTo(Address to)
{
    _to = std::move(to);
}


void TransactionBuilder::setAmount(Balance amount)
{
    _amount = amount;
}


void TransactionBuilder::setTimestamp(base::Time timestamp)
{
    _timestamp = timestamp;
}


void TransactionBuilder::setFee(Fee fee)
{
    _fee = fee;
}


void TransactionBuilder::setSign(Sign sign)
{
    _sign = std::move(sign);
}


void TransactionBuilder::setData(base::Bytes data)
{
    _data = std::move(data);
}


Transaction TransactionBuilder::build() const&
{
    ASSERT(_from);
    ASSERT(_to);
    ASSERT(_amount);
    ASSERT(_fee);
    ASSERT(_timestamp);
    ASSERT(_data);

    if (_sign) {
        return { *_from, *_to, *_amount, *_fee, *_timestamp, *_data, *_sign };
    }
    else {
        return { *_from, *_to, *_amount, *_fee, *_timestamp, *_data };
    }
}


Transaction TransactionBuilder::build() &&
{
    ASSERT(_from);
    ASSERT(_to);
    ASSERT(_amount);
    ASSERT(_fee);
    ASSERT(_timestamp);
    ASSERT(_data);

    if (_sign) {
        return { std::move(*_from),      std::move(*_to),   std::move(*_amount), std::move(*_fee),
                 std::move(*_timestamp), std::move(*_data), std::move(*_sign) };
    }
    else {
        return { std::move(*_from), std::move(*_to),        std::move(*_amount),
                 std::move(*_fee),  std::move(*_timestamp), std::move(*_data) };
    }
}


const Transaction& invalidTransaction()
{
    static const Transaction invalid_tx{ Address::null(), Address::null(), 0, 1, base::Time(), {} };
    return invalid_tx;
}


TransactionStatus::TransactionStatus(StatusCode status,
                                     ActionType type,
                                     Fee fee_left,
                                     const std::string& message) noexcept
  : _status{ status }
  , _action(type)
  , _message{ message }
  , _fee_left{ fee_left }
{}


TransactionStatus::operator bool() const noexcept
{
    return _status == TransactionStatus::StatusCode::Success;
}


bool TransactionStatus::operator!() const noexcept
{
    return _status != TransactionStatus::StatusCode::Success;
}


const std::string& TransactionStatus::getMessage() const noexcept
{
    return _message;
}

std::string& TransactionStatus::getMessage() noexcept
{
    return _message;
}


TransactionStatus::StatusCode TransactionStatus::getStatus() const noexcept
{
    return _status;
}


TransactionStatus::ActionType TransactionStatus::getType() const noexcept
{
    return _action;
}

Fee TransactionStatus::getFeeLeft() const noexcept
{
    return _fee_left;
}

} // namespace lk
