#include "balance_manager.hpp"

#include "base/error.hpp"

namespace bc
{

BalanceManager::BalanceManager(const std::map<Address, Balance>& initial_state) : _storage(initial_state)
{}

bool BalanceManager::checkTransaction(const Transaction& tx)
{
    auto from_iter = _storage.find(tx.getFrom());
    if(from_iter != _storage.end()) {
        return from_iter->second < tx.getAmount();
    }
    else {
        return false;
    }
}

void BalanceManager::update(const Transaction& tx)
{
    auto from_iter = _storage.find(tx.getFrom());
    if(from_iter != _storage.end()) {
        if(from_iter->second >= tx.getAmount()) {
            from_iter->second = from_iter->second - tx.getAmount();
        }
        else {
            RAISE_ERROR(base::InvalidArgument, "From address has insufficient money");
        }
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "There is no information on from address in the blockchain");
    }

    auto to_iter = _storage.find(tx.getTo());
    if(to_iter != _storage.end()) {
        to_iter->second = to_iter->second + tx.getAmount();
    }
    else {
        _storage.insert(std::pair<Address, Balance>(tx.getTo(), tx.getAmount()));
    }
}

} // namespace bc