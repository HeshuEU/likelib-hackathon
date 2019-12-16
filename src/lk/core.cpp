#include "core.hpp"

namespace lk
{

Core::Core(const base::PropertyTree& config) : _config{config}, _network{_config, *this}
{
    applyGenesis();

    signal_new_block.connect(_blockchain.signal_block_added);
}


const bc::Block& Core::getGenesisBlock()
{
    static bc::Block genesis = [] {
        bc::Block ret{base::Sha256(base::Bytes(32)), {}};
        bc::Address null_address(bc::Address(std::string(32, '0')));
        ret.addTransaction({null_address, null_address, bc::Balance{0xFFFFFFFF}, base::Time::fromSeconds(0)});
        return ret;
    }();
    return genesis;
}


void Core::applyGenesis()
{
    ASSERT(_blockchain.tryAddBlock(getGenesisBlock()));
    _balance_manager.updateFromGenesis(getGenesisBlock());
}


void Core::run()
{
    _network.run();
}


bool Core::tryAddBlock(const bc::Block& b)
{
    if(checkBlock(b) && _blockchain.tryAddBlock(b)) {
        _pending_transactions.remove(b.getTransactions());
        _balance_manager.update(b);
        _network.broadcastBlock(b);
        signal_new_block(b);
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
        if(!_balance_manager.checkTransaction(tx)) {
            return false;
        }
    }

    return true;
}


bc::Block Core::getBlockTemplate() const
{
    auto prev_hash = base::Sha256::compute(base::toBytes(_blockchain.getTopBlock()));
    return bc::Block{prev_hash, _pending_transactions};
}


bool Core::performTransaction(const bc::Transaction& tx)
{
    if(_blockchain.findTransaction(base::Sha256::compute(base::toBytes(tx)))) {
        return false;
    }
    else if(_pending_transactions.find(tx)) {
        return false;
    }
    else {
        _pending_transactions.add(tx);
        _network.broadcastTransaction(tx);
        signal_new_transaction(tx);
        return true;
    }
}


bc::Balance Core::getBalance(const bc::Address& address) const
{
    return _balance_manager.getBalance(address);
}


const bc::Block& Core::getTopBlock() const
{
    return _blockchain.getTopBlock();
}

} // namespace lk
