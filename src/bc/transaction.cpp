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


void TransactionBuilder::setFrom(const bc::Address& from)
{
    _from = from;
}


void TransactionBuilder::setTo(const bc::Address& to)
{
    _to = to;
}


void TransactionBuilder::setAmount(const bc::Balance& amount)
{
    _amount = amount;
}


Transaction TransactionBuilder::build()
{
    return {_from, _to, _amount};
}

} // namespace bc