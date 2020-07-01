#include "transactions_set.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

namespace lk
{

void TransactionsSet::add(const lk::Transaction& tx)
{
    if (!find(tx)) {
        _txs.push_back(tx);
    }
}


bool TransactionsSet::find(const lk::Transaction& tx) const
{
    return std::find(_txs.begin(), _txs.end(), tx) != _txs.end();
}


std::optional<Transaction> TransactionsSet::find(const base::Sha256& hash) const
{
    auto it =
      std::find_if(_txs.begin(), _txs.end(), [&hash](const auto& tx) { return tx.hashOfTransaction() == hash; });

    if (it == _txs.end()) {
        return std::nullopt;
    }
    else {
        return *it;
    }
}


void TransactionsSet::remove(const lk::Transaction& tx)
{
    auto it = std::find(_txs.begin(), _txs.end(), tx);
    if (it == _txs.end()) {
        return;
    }

    auto last_vector_element = _txs.end();
    std::advance(last_vector_element, -1);

    if (last_vector_element != it) {
        *it = std::move(*last_vector_element);
    }
    _txs.pop_back();
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
    for (const auto& tx : other) {
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


void TransactionsSet::selectBestByFee(std::size_t n)
{
    if (n > size()) {
        RAISE_ERROR(base::InvalidArgument, "cannot select more transactions than there are in set");
    }

    std::nth_element(
      _txs.begin(), _txs.begin() + n, _txs.end(), [](const auto& a, const auto& b) { return a.getFee() > b.getFee(); });

    _txs.erase(_txs.begin() + n, _txs.end());
}


void TransactionsSet::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_txs);
}


TransactionsSet TransactionsSet::deserialize(base::SerializationIArchive& ia)
{
    TransactionsSet tx_set;
    tx_set._txs = ia.deserialize<std::vector<lk::Transaction>>();
    return tx_set;
}


std::map<Address, Balance> calcCost(const TransactionsSet& txs)
{
    std::map<Address, Balance> result;
    for (const auto& tx : txs) {
        auto tx_cost = tx.getAmount() + tx.getFee();
        result.insert({ tx.getFrom(), tx_cost });
    }
    return result;
}

} // namespace lk
