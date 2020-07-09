#pragma once

#include "core/transaction.hpp"

#include "base/serialization.hpp"

#include <map>
#include <vector>

namespace lk
{

class TransactionsSet
{
  public:
    //=================
    TransactionsSet() = default;
    //=================
    void add(const Transaction& tx);
    [[nodiscard]] bool find(const Transaction& tx) const;
    [[nodiscard]] std::optional<Transaction> find(const base::Sha256& hash) const;
    void remove(const Transaction& tx);
    void remove(const TransactionsSet& other);
    bool isEmpty() const;

    std::size_t size() const;

    std::vector<Transaction>::const_iterator begin() const;
    std::vector<Transaction>::const_iterator end() const;

    std::vector<Transaction>::iterator begin();
    std::vector<Transaction>::iterator end();

    bool operator==(const TransactionsSet& other) const;
    bool operator!=(const TransactionsSet& other) const;
    //=================
    void selectBestByFee(std::size_t n);
    //=================
    void serialize(base::SerializationOArchive& oa) const;
    static TransactionsSet deserialize(base::SerializationIArchive& ia);
    //=================
  private:
    std::vector<Transaction> _txs;
};


std::map<Address, Balance> calcCost(const TransactionsSet& txs);

} // namespace lk