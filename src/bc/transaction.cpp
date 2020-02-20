#include "base/error.hpp"

#include "transaction.hpp"

namespace bc
{

Transaction::Transaction(
    const bc::Address& from, const bc::Address& to, const bc::Balance& amount, const base::Time& timestamp)
    : _from{from}, _to{to}, _amount{amount}, _timestamp(timestamp)
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


void Transaction::setFrom(const bc::Address& from)
{
    _from = from;
}


void Transaction::setTo(const bc::Address& to)
{
    _to = to;
}


void Transaction::setAmount(const bc::Balance& amount)
{
    if(amount == 0) {
        RAISE_ERROR(base::LogicError, "Transaction cannot contain amount equal to 0");
    }
    _amount = amount;
}


void Transaction::setTimestamp(const base::Time& timestamp)
{
    _timestamp = timestamp;
}


bool Transaction::operator==(const Transaction& other) const
{
    return _amount == other._amount && _from == other._from && _to == other._to && _timestamp == other._timestamp;
}


bool Transaction::operator!=(const Transaction& other) const
{
    return !(*this == other);
}


base::SerializationOArchive& Transaction::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(getFrom());
    oa.serialize(getTo());
    oa.serialize(getAmount());
    oa.serialize(getTimestamp());
    return oa;
}


Transaction Transaction::deserialize(base::SerializationIArchive& ia)
{
    auto from = ia.deserialize<bc::Address>();
    auto to = ia.deserialize<bc::Address>();
    auto balance = ia.deserialize<bc::Balance>();
    auto timestamp = ia.deserialize<base::Time>();
    return Transaction(from, to, balance, timestamp);
}


std::ostream& operator<<(std::ostream& os, const Transaction& tx)
{
    return os << "from: " << tx.getFrom() << " to: " << tx.getTo() << " amount: " << tx.getAmount()
              << " timestamp: " << tx.getTimestamp();
}

} // namespace bc
