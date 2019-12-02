#pragma once

#include "base/serialization.hpp"
#include "bc/transaction.hpp"

#include <vector>

namespace bc
{

class TransactionsSet
{
  public:
    void add(const Transaction& tx);
    bool find(const Transaction& tx) const;
    void remove(const Transaction& tx);
    void remove(const TransactionsSet& other);
    bool isEmpty() const;

    std::vector<Transaction>::const_iterator begin() const;
    std::vector<Transaction>::const_iterator end() const;

    std::vector<Transaction>::iterator begin();
    std::vector<Transaction>::iterator end();

  private:
    std::vector<Transaction> _txs;

    friend base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, TransactionsSet& txs);
    friend base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const TransactionsSet& txs);
};

} // namespace bc