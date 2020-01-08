#pragma once

#include "base/serialization.hpp"
#include "bc/transaction.hpp"

#include <vector>
#include <map>

namespace bc
{

class TransactionsSet
{
  public:
    void add(const Transaction& tx);
    [[nodiscard]] bool find(const Transaction& tx) const;
    [[nodiscard]] std::optional<Transaction> find(const base::Sha256& hash) const;
    void remove(const Transaction& tx);
    void remove(const TransactionsSet& other);
    [[nodiscard]] bool isEmpty() const;

    [[nodiscard]] std::size_t size() const;

    [[nodiscard]] std::vector<Transaction>::const_iterator begin() const;
    [[nodiscard]] std::vector<Transaction>::const_iterator end() const;

    [[nodiscard]] std::vector<Transaction>::iterator begin();
    [[nodiscard]] std::vector<Transaction>::iterator end();

  private:
    std::vector<Transaction> _txs;

    friend base::SerializationIArchive& operator>>(base::SerializationIArchive& ia, TransactionsSet& txs);
    friend base::SerializationOArchive& operator<<(base::SerializationOArchive& oa, const TransactionsSet& txs);
};


std::map<Address, Balance> calcBalance(const TransactionsSet& txs);

} // namespace bc