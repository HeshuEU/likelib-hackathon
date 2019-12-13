#include "core.hpp"

namespace lk
{

Core::Core(const base::PropertyTree& config) : _config{config}, _protocol_engine{_config, _blockchain}
{
    applyGenesis();

    signal_new_block.connect(_blockchain.signal_block_added);
    signal_new_transaction.connect(_protocol_engine.signal_transaction_received);
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
    _protocol_engine.run();
}


void Core::tryAddBlock(const bc::Block& b)
{
    if(_blockchain.tryAddBlock(b))
    {
        _protocol_engine.broadcastBlock(b);
    }
}


} // namespace lk
