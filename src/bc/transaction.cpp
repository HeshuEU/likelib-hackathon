#include "base/error.hpp"

#include "transaction.hpp"

namespace bc
{

Transaction::Transaction(bc::Address from, bc::Address to, bc::Balance amount, base::Time timestamp)
    : _from{std::move(from)}, _to{std::move(to)}, _amount{amount}, _timestamp{timestamp}
{
    if(_amount == 0) {
        RAISE_ERROR(base::LogicError, "Transaction cannot contain amount equal to 0");
    }
}


const bc::Address& Transaction::getFrom() const noexcept
{
    return _from;
}


const bc::Address& Transaction::getTo() const noexcept
{
    return _to;
}


const bc::Balance& Transaction::getAmount() const noexcept
{
    return _amount;
}


const base::Time& Transaction::getTimestamp() const noexcept
{
    return _timestamp;
}



bool Transaction::operator==(const Transaction& other) const
{
    return _amount == other._amount && _from == other._from && _to == other._to && _timestamp == other._timestamp;
}


bool Transaction::operator!=(const Transaction& other) const
{
    return !(*this == other);
}


void Transaction::sign(const base::RsaPrivateKey& priv)
{
    auto hash = hashOfTxData();
    _sign = priv.encrypt(hash.getBytes());
}


bool Transaction::checkSign(const base::RsaPublicKey& pub) const
{
    if(_sign.isEmpty()) {
        return false;
    }
    auto hash = hashOfTxData();
    return hash == pub.decrypt(_sign);
}


std::optional<base::Bytes> Transaction::getSign() const
{
    if(_sign.isEmpty()) {
        return std::nullopt;
    }
    else {
        return _sign;
    }
}


base::Sha256 Transaction::hashOfTxData() const
{
    base::SerializationOArchive oa;
    oa << _from << _to << _amount << _timestamp;
    return base::Sha256::compute(std::move(oa).getBytes());
}


Transaction Transaction::deserialize(base::SerializationIArchive& ia)
{
    auto from = bc::Address::deserialize(ia);
    auto to = bc::Address::deserialize(ia);
    bc::Balance balance;
    ia >> balance;
    ::base::Time timestamp;
    ia >> timestamp;
    return {std::move(from), std::move(to), balance, timestamp};
}


void Transaction::serialize(base::SerializationOArchive& oa) const
{
    oa << _from << _to << _amount << _timestamp;
}


std::ostream& operator<<(std::ostream& os, const Transaction& tx)
{
    return os << "from: " << tx.getFrom() << " to: " << tx.getTo() << " amount: " << tx.getAmount()
              << " timestamp: " << tx.getTimestamp();
}


void TransactionBuilder::setFrom(bc::Address from)
{
    _from = std::move(from);
}


void TransactionBuilder::setTo(bc::Address to)
{
    _to = std::move(to);
}


void TransactionBuilder::setAmount(bc::Balance amount)
{
    _amount = amount;
}


void TransactionBuilder::setTimestamp(base::Time timestamp)
{
    _timestamp = timestamp;
}


Transaction TransactionBuilder::build() const&
{
    ASSERT(_from);
    ASSERT(_to);
    ASSERT(_amount);
    ASSERT(_timestamp);
    return {*_from, *_to, *_amount, *_timestamp};
}


Transaction TransactionBuilder::build() &&
{
    ASSERT(_from);
    ASSERT(_to);
    ASSERT(_amount);
    ASSERT(_timestamp);
    return {std::move(*_from), std::move(*_to), std::move(*_amount), std::move(*_timestamp)};
}


} // namespace bc
