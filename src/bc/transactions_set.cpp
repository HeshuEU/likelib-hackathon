#include "transactions_set.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

namespace bc
{

void TransactionsSet::add(const bc::Transaction& tx)
{
    if(!find(tx)) {
        _txs.push_back(tx);
    }
}


bool TransactionsSet::find(const bc::Transaction& tx) const
{
    return std::find(_txs.begin(), _txs.end(), tx) != _txs.end();
}


std::optional<Transaction> TransactionsSet::find(const base::Sha256& hash) const
{
    auto it = std::find_if(_txs.begin(), _txs.end(), [&hash](const auto& tx) {
        return base::Sha256::compute(base::toBytes(tx)) == hash;
    });

    if(it == _txs.end()) {
        return std::nullopt;
    }
    else {
        return *it;
    }
}


void TransactionsSet::remove(const bc::Transaction& tx)
{
    auto it = std::find(_txs.begin(), _txs.end(), tx);
    if(it == _txs.end()) {
        return;
    }

    auto last_vector_element = _txs.end();
    std::advance(last_vector_element, -1);

    if(last_vector_element != it) {
        *it = std::move(*last_vector_element);
    }
    _txs.resize(_txs.size() - 1);
}


bool TransactionsSet::isEmpty() const
{
    return _txs.empty();
}


std::size_t TransactionsSet::size() const
{
    return _txs.size();
}


std::vector<Transaction>::const_iterator TransactionsSet::begin() const
{
    return _txs.begin();
}


std::vector<Transaction>::const_iterator TransactionsSet::end() const
{
    return _txs.end();
}


std::vector<Transaction>::iterator TransactionsSet::begin()
{
    return _txs.begin();
}


std::vector<Transaction>::iterator TransactionsSet::end()
{
    return _txs.end();
}


void TransactionsSet::remove(const TransactionsSet& other)
{
    for(const auto& tx: other) {
        remove(tx);
    }
}


bool TransactionsSet::operator==(const TransactionsSet& other) const
{
    return _txs == other._txs;
}


bool TransactionsSet::operator!=(const TransactionsSet& other) const
{
    return !(*this == other);
}


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, TransactionsSet& txs)
{
    return ia >> txs._txs;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const TransactionsSet& txs)
{
    return oa << txs._txs;
}

} // namespace bc
