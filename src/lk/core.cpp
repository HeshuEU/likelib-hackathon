#include "core.hpp"

#include "base/log.hpp"
#include "vm/host.hpp"


namespace
{

bc::Address ethAddressToNative(const evmc::address& addr)
{
    base::Bytes raw_address(addr.bytes, 20);
    bc::Address address(raw_address);
    return address;
}

}


namespace lk
{

Core::Core(const base::PropertyTree& config, const base::KeyVault& key_vault)
    : _config{config}, _vault{key_vault}, _blockchain{_config}, _network{_config, *this},
      _vm{vm::VM::load(*this)}
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
                            base::Time::fromSecondsSinceEpochBeginning(0), base::Sha256::null()});
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


vm::VM& Core::getVm()
{
    return _vm;
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


//=====================================

bool Core::account_exists(const evmc::address& addr) const noexcept
{
    auto address = ethAddressToNative(addr);
    return _account_manager.hasAccount(address)
}


evmc::bytes32 Core::get_storage(const evmc::address& addr, const evmc::bytes32& ethKey) const noexcept
{
    auto address = ethAddressToNative(addr);
    base::Bytes key(ethKey.bytes, 32);
    return _account_manager.getAccount(address).getStorageValue(key);
}


evmc_storage_status Core::set_storage(
        const evmc::address& addr, const evmc::bytes32& key, const evmc::bytes32& value) noexcept
{}


evmc::uint256be Core::get_balance(const evmc::address& addr) const noexcept
{
    auto address = ethAddressToNative(addr);
    auto balance = getBalance(address);
    evmc::uint256be ret;
    for(int i = 0; i < 32; ++i) ret.bytes[i] = 0;
    for(int i = sizeof(balance) * 8; i >= 0; --i) {
        ret.bytes[i] = balance & 0xFF;
        balance >>= 8;
    }
    return ret;
}


size_t Core::get_code_size(const evmc::address& addr) const noexcept
{
    auto address = ethAddressToNative(addr);
    
}


evmc::bytes32 Core::get_code_hash(const evmc::address& addr) const noexcept
{}


size_t Core::copy_code(
        const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const noexcept
{}


void Core::selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept
{}


evmc::result Core::call(const evmc_message& msg) noexcept
{}


evmc_tx_context Core::get_tx_context() const noexcept
{}


evmc::bytes32 Core::get_block_hash(int64_t block_number) const noexcept
{}


void Core::emit_log(const evmc::address& addr, const uint8_t* data, size_t data_size,
                                  const evmc::bytes32 topics[], size_t num_topics) noexcept
{}

//===========================


} // namespace lk
