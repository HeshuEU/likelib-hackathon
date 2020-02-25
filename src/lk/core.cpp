#include "core.hpp"

#include "base/log.hpp"
#include "vm/host.hpp"

namespace lk
{

Core::Core(const base::PropertyTree& config, const base::KeyVault& key_vault)
    : _config{config}, _vault{key_vault}, _blockchain{_config}, _network{_config, *this},
      _host_impl{}, _vm{vm::VM::load(_host_impl)}
{
    [[maybe_unused]] bool result = _blockchain.tryAddBlock(getGenesisBlock());
    ASSERT(result);
    _account_manager.updateFromGenesis(getGenesisBlock());
    _blockchain.load();
}


const bc::Block& Core::getGenesisBlock()
{
    static bc::Block genesis = [] {
        bc::Block ret{0, base::Sha256(base::Bytes(32)), {}};
        bc::Address null_address;
        ret.addTransaction({null_address, bc::Address{"UTpE8/SckOrfV4Fn/Gi3jmLEOVI="}, bc::Balance{0xFFFFFFFF}, 0,
            base::Time::fromSecondsSinceEpochBeginning(0)});
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
    if(checkBlock(b) && _blockchain.tryAddBlock(b)) {
        {
            std::shared_lock lk(_pending_transactions_mutex);
            _pending_transactions.remove(b.getTransactions());
        }
        updateNewBlock(b);
        _event_block_added.notify(b);
        return true;
    }
    else {
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
        if(!_account_manager.checkTransaction(tx)) {
            return false;
        }
    }

    return true;
}


bool Core::checkTransaction(const bc::Transaction& tx) const
{
    if(!tx.checkSign()) {
        return false;
    }

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
        auto current_from_account_balance = _account_manager.getBalance(tx.getFrom());
        return pending_from_account_balance->second + current_from_account_balance >= tx.getAmount();
    }
    else {
        return _account_manager.checkTransaction(tx);
    }
}


bc::Block Core::getBlockTemplate() const
{
    const auto& top_block = _blockchain.getTopBlock();
    bc::BlockDepth depth = top_block.getDepth() + 1;
    auto prev_hash = base::Sha256::compute(base::toBytes(top_block));
    std::shared_lock lk(_pending_transactions_mutex);
    return bc::Block{depth, prev_hash, _pending_transactions};
}


bool Core::performTransaction(const bc::Transaction& tx)
{
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
    return _account_manager.getBalance(address);
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
    if(!_is_account_manager_updated) {
        _account_manager.updateFromGenesis(block);
        _is_account_manager_updated = true;
    }
    else {
        _account_manager.update(block);
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
