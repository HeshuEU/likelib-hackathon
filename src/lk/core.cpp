#include "core.hpp"

namespace lk
{

Core::Core(const base::PropertyTree& config) : _config{config}, _network{_config, *this}
{
    applyGenesis();

    signal_new_block.connect(_blockchain.signal_block_added);
    //signal_new_transaction.connect(_network.signal_transaction_received);
}


const bc::Block& Core::getGenesisBlock()
{
    static bc::Block genesis = [] {
        bc::Block ret;
        ret.setPrevBlockHash(base::Bytes(32));
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


bool Core::performTransaction(const bc::Transaction& tx)
{
    if(_blockchain.findTransaction(base::Sha256::compute(base::toBytes(tx)))) {
        return true;
    }
    else {
        _network.broadcastTransaction(tx);
        signal_new_transaction(tx);
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
