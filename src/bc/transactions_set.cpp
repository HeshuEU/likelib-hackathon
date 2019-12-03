#include "transactions_set.hpp"

#include "base/assert.hpp"
#include "base/error.hpp"

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
    auto a = _txs.begin();
    auto b = _txs.end();
    auto it = std::find(a, b, tx);
    return it != b;
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


base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, TransactionsSet& txs)
{
    return ia >> txs._txs;
}


base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const TransactionsSet& txs)
{
    return oa << txs._txs;
}

} // namespace bc
