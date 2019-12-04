#include "transaction.hpp"

namespace bc
{

Transaction::Transaction(
    const bc::Address& from, const bc::Address& to, const bc::Balance& amount, const base::Time& timestamp)
    : _from{from}, _to{to}, _amount{amount}, _timestamp(timestamp)
{}


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


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, Transaction& tx)
{
    bc::Address from, to;
    ia >> from >> to;
    tx.setFrom(from);
    tx.setTo(to);
    bc::Balance balance;
    ia >> balance;
    tx.setAmount(balance);
    ::base::Time timestamp;
    ia >> timestamp;
    tx.setTimestamp(timestamp);
    return ia;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const Transaction& tx)
{
    return oa << tx.getFrom() << tx.getTo() << tx.getAmount() << tx.getTimestamp();
}


void removeProcessedTransactions(std::vector<Transaction>& txs, const std::vector<Transaction>& processed)
{
    txs.erase(std::remove_if(txs.begin(), txs.end(),
                  [&processed](const Transaction& tx) {
                      return std::find(processed.begin(), processed.end(), tx) != processed.end();
                  }),
        txs.end());
}

} // namespace bc
