#include "core.hpp"

#include "base/log.hpp"

namespace lk
{

Core::Core(const base::PropertyTree& config, const base::KeyVault& key_vault)
    : _config{config}, _blockchain{_config}, _network{_config, *this}, _vault{key_vault}
{
    LOG_CURRENT_FUNCTION;
    [[maybe_unused]] bool result = _blockchain.tryAddBlock(getGenesisBlock());
    ASSERT(result);
    _balance_manager.updateFromGenesis(getGenesisBlock());
    _blockchain.load();
}


const bc::Block& Core::getGenesisBlock()
{
    static bc::Block genesis = [] {
        bc::Block ret{0, base::Sha256(base::Bytes(32)), {}};
        bc::Address null_address(bc::Address(std::string(32, '0')));
        ret.addTransaction(
            {null_address, null_address, bc::Balance{0xFFFFFFFF}, base::Time::fromSecondsSinceEpochBeginning(0)});
        return ret;
    }();
    return genesis;
}


void Core::run()
{
    _network.run();
}


bool Core::tryAddBlock(const bc::Block& b)
{
    LOG_CURRENT_FUNCTION << " with block = " << b;
    if(checkBlock(b) && _blockchain.tryAddBlock(b)) {
        LOG_CURRENT_FUNCTION << "removing txs in block from pending";
        {
            LOG_CURRENT_FUNCTION << "acquiring shared lock";
            std::shared_lock lk(_pending_transactions_mutex);
            LOG_CURRENT_FUNCTION << "removing txs";
            _pending_transactions.remove(b.getTransactions());
            LOG_CURRENT_FUNCTION << "removed transactions and freed lock";
        }
        LOG_CURRENT_FUNCTION << "updating during to a new block";
        updateNewBlock(b);
        LOG_CURRENT_FUNCTION << "notifying all subscribers to a new block";
        _event_block_added.notify(b);
        return true;
    }
    else {
        LOG_CURRENT_FUNCTION << "rejecting this block";
        return false;
    }
}


std::optional<bc::Block> Core::findBlock(const base::Sha256& hash) const
{
    return _blockchain.findBlock(hash);
}


bool Core::checkBlock(const bc::Block& b) const
{
    if(_blockchain.findBlock(base::Sha256::compute(base::toBytes(b)))) {
        return false;
    }

    // FIXME: this works wrong if two transactions are both valid, but together are not
    for(const auto& tx: b.getTransactions()) {
        if(!_balance_manager.checkTransaction(tx)) {
            return false;
        }
    }

    return true;
}


bool Core::checkTransaction(const bc::Transaction& tx) const
{
    if(_blockchain.findTransaction(base::Sha256::compute(base::toBytes(tx)))) {
        return false;
    }

    std::map<bc::Address, bc::Balance> current_pending_balance;
    {
        std::shared_lock lk(_pending_transactions_mutex);
        if(_pending_transactions.find(tx)) {
            return false;
        }

        current_pending_balance = bc::calcBalance(_pending_transactions);
    }

    auto pending_from_account_balance = current_pending_balance.find(tx.getFrom());
    if(pending_from_account_balance != current_pending_balance.end()) {
        auto current_from_account_balance = _balance_manager.getBalance(tx.getFrom());
        return ((pending_from_account_balance->second + current_from_account_balance - tx.getAmount()) >= 0);
    }
    else {
        return _balance_manager.checkTransaction(tx);
    }
}


bc::Block Core::getBlockTemplate() const
{
    LOG_CURRENT_FUNCTION;
    LOG_CURRENT_FUNCTION << "retrieving ours top block";
    const auto& top_block = _blockchain.getTopBlock();
    bc::BlockDepth depth = top_block.getDepth() + 1;
    auto prev_hash = base::Sha256::compute(base::toBytes(top_block));
    LOG_CURRENT_FUNCTION << "acquiring lock on pending txs";
    std::shared_lock lk(_pending_transactions_mutex);
    LOG_CURRENT_FUNCTION << "returning block";
    return bc::Block{depth, prev_hash, _pending_transactions};
}


bool Core::performTransaction(const bc::Transaction& tx)
{
    LOG_CURRENT_FUNCTION << tx;
    if(checkTransaction(tx)) {
        {
            std::unique_lock lk(_pending_transactions_mutex);
            _pending_transactions.add(tx);
        }
        _event_new_pending_transaction.notify(tx);
        return true;
    }
    return false;
}


bc::Balance Core::getBalance(const bc::Address& address) const
{
    LOG_CURRENT_FUNCTION;
    return _balance_manager.getBalance(address);
}


const bc::Block& Core::getTopBlock() const
{
    return _blockchain.getTopBlock();
}


base::Bytes Core::getThisNodeAddress() const
{
    return _vault.getPublicKey().toBytes();
}


void Core::updateNewBlock(const bc::Block& block)
{
    LOG_CURRENT_FUNCTION << block;
    if(!_is_balance_manager_updated) {
        _balance_manager.updateFromGenesis(block);
        _is_balance_manager_updated = true;
    }
    else {
        _balance_manager.update(block);
    }
}


void Core::subscribeToBlockAddition(decltype(Core::_event_block_added)::CallbackType callback)
{
    _event_block_added.subscribe(std::move(callback));
}



void Core::subscribeToNewPendingTransaction(decltype(Core::_event_new_pending_transaction)::CallbackType callback)
{
    _event_new_pending_transaction.subscribe(std::move(callback));
}


} // namespace lk
