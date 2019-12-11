#include "core.hpp"

namespace lk
{

Core::Core(const base::PropertyTree& config) : _config{config}, _host{config}
{
    applyGenesis();
}


const bc::Block& getGenesisBlock()
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
    _host.run();
}


} // namespace lk
