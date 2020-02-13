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


Transaction Transaction::deserialize(base::SerializationIArchive &ia)
{
    bc::Address from, to;
    ia >> from >> to;
    bc::Balance balance;
    ia >> balance;
    ::base::Time timestamp;
    ia >> timestamp;
    return {std::move(from), std::move(to), balance, timestamp};
}


void Transaction::serialize(base::SerializationOArchive &oa) const
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


Transaction TransactionBuilder::build() const &
{
    return {_from, _to, _amount, _timestamp};
}


Transaction TransactionBuilder::build() &&
{
    return {_from, _to, _amount, _timestamp};
}


} // namespace bc
