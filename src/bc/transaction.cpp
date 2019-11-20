#include "transaction.hpp"

namespace bc
{

Transaction::Transaction(const bc::Address& from, const bc::Address& to, const bc::Balance& amount)
    : _from{from}, _to{to}, _amount{amount}
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


base::SerializationIArchive operator>>(base::SerializationIArchive& ia, Transaction& tx)
{
    bc::Balance balance;
    ia >> balance;
    tx.setAmount(balance);
    return ia;
}


base::SerializationOArchive operator<<(base::SerializationOArchive& oa, const Transaction& tx)
{
    return oa << tx.getAmount(); //tx.getFrom() << tx.getTo();
}

} // namespace bc