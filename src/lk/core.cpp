#include "core.hpp"

#include "base/log.hpp"
#include "vm/host.hpp"
#include "vm/tools.hpp"

#include <algorithm>
#include <iterator>


namespace lk
{

Core::Core(const base::PropertyTree& config, const base::KeyVault& key_vault)
    : _config{config}, _vault{key_vault}, _blockchain{_config}, _network{_config, *this}
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
            base::Time::fromSecondsSinceEpochBeginning(0), base::Sha256::null(), bc::Transaction::Type::MESSAGE_CALL,
            base::Bytes{}});
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
        auto current_from_account_balance = _account_manager.getAccount(tx.getFrom()).getBalance();
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
    ASSERT(tx.getType() == bc::Transaction::Type::MESSAGE_CALL);

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
    return _account_manager.getAccount(address).getBalance();
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
        for(const auto& tx : block.getTransactions()) {
            if(tx.getType() == bc::Transaction::Type::CONTRACT_CREATION) {
                doContractCreation(tx);
            }
            else if(tx.getType() == bc::Transaction::Type::MESSAGE_CALL) {
                doMessageCall(tx);
            }
            else {
                // TODO: do something, not just throw an exception, because state must be reverted in that case
            }
        }
    }
}


void Core::doContractCreation(const bc::Transaction& tx)
{
    auto vm = vm::Vm::load(*this);
    vm::SmartContract contract(tx.getData()); // TEMPORARY!!!!!11111
    auto message = contract.createInitMessage(tx.getFee(), tx.getFrom(), ----, tx.getAmount(), -----);
    //auto result = vm.execute(message);
}


void Core::doMessageCall(const bc::Transaction& tx)
{
    if(tx.getCodeHash() == base::Sha256::null()) {
        // just transfer
        _account_manager.update(tx);
        return;
    }

    // if we're here -- do a call to a contract
    auto vm = vm::Vm::load(*this);
    vm::SmartContract contract(_account_manager.getCode(tx.getCodeHash()));
    auto message = contract.createMessage(tx.getFee(), tx.getFrom(), tx.getTo(), tx.getAmount(), tx.getData());
    auto result = vm.execute(message);
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
    LOG_DEBUG << "Core::account_exists";
    auto address = vm::toNativeAddress(addr);
    return _account_manager.hasAccount(address);
}


evmc::bytes32 Core::get_storage(const evmc::address& addr, const evmc::bytes32& ethKey) const noexcept
{
    LOG_DEBUG << "Core::get_storage";
    auto address = vm::toNativeAddress(addr);
    base::Bytes key(ethKey.bytes, 32);
    auto storage_value = _account_manager.getAccount(address).getStorageValue(key).data;
    return vm::toEvmcBytes32(storage_value);
}


evmc_storage_status Core::set_storage(
    const evmc::address& addr, const evmc::bytes32& ekey, const evmc::bytes32& evalue) noexcept
{
    LOG_DEBUG << "Core::set_storage";
    static const base::Bytes NULL_VALUE(32);
    auto address = vm::toNativeAddress(addr);
    base::Bytes key(ekey.bytes, 32);
    base::Bytes new_value(evalue.bytes, 32);

    auto& account_state = _account_manager.getAccount(address);

    if(!account_state.checkStorageValue(key)) {
        if(new_value == NULL_VALUE) {
            return evmc_storage_status::EVMC_STORAGE_UNCHANGED;
        }
        else {
            account_state.setStorageValue(key, new_value);
            return evmc_storage_status::EVMC_STORAGE_ADDED;
        }
    }
    else {
        auto old_storage_data = account_state.getStorageValue(key);
        const auto& old_value = old_storage_data.data;

        account_state.setStorageValue(key, new_value);
        if(old_value == new_value) {
            return evmc_storage_status::EVMC_STORAGE_UNCHANGED;
        }
        else if(new_value == NULL_VALUE) {
            return evmc_storage_status::EVMC_STORAGE_DELETED;
        }
        else {
            return evmc_storage_status::EVMC_STORAGE_MODIFIED;
        }
    }
}


evmc::uint256be Core::get_balance(const evmc::address& addr) const noexcept
{
    LOG_DEBUG << "Core::get_balance";
    auto address = vm::toNativeAddress(addr);
    auto balance = getBalance(address);
    evmc::uint256be ret;
    std::fill(std::begin(ret.bytes), std::end(ret.bytes), 0);
    for(int i = sizeof(balance) * 8; i >= 0; --i) {
        ret.bytes[i] = balance & 0xFF;
        balance >>= 8;
    }
    return ret;
}


size_t Core::get_code_size(const evmc::address& addr) const noexcept
{
    LOG_DEBUG << "Core::get_code_size";
    auto address = vm::toNativeAddress(addr);
    auto account_code_hash = _account_manager.getAccount(address).getCodeHash();
    return _account_manager.getCode(account_code_hash).size();
}


evmc::bytes32 Core::get_code_hash(const evmc::address& addr) const noexcept
{
    LOG_DEBUG << "Core::get_code_hash";
    auto address = vm::toNativeAddress(addr);
    auto account_code_hash = _account_manager.getAccount(address).getCodeHash();
    return vm::toEvmcBytes32(account_code_hash.getBytes());
}


size_t Core::copy_code(const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const
    noexcept
{
    LOG_DEBUG << "Core::copy_code";
    auto address = vm::toNativeAddress(addr);
    auto account_code_hash = _account_manager.getAccount(address).getCodeHash();
    const auto& code = _account_manager.getCode(account_code_hash);
    std::size_t bytes_to_copy = std::min(buffer_size, code.size() - code_offset);
    std::copy(code.toArray() + code_offset, code.toArray() + code_offset + bytes_to_copy, buffer_data);
    return bytes_to_copy;
}


void Core::selfdestruct(const evmc::address& eaddr, const evmc::address& ebeneficiary) noexcept
{
    LOG_DEBUG << "Core::selfdestruct";
    auto address = vm::toNativeAddress(eaddr);
    auto account = _account_manager.getAccount(address);
    auto beneficiary_address = vm::toNativeAddress(ebeneficiary);
    auto beneficiary_account = _account_manager.getAccount(beneficiary_address);

    // TODO: transfer the rest of money to a beneficiary account
}


evmc::result Core::call(const evmc_message& msg) noexcept
{
    LOG_DEBUG << "Core::call";
    bc::Balance fee = msg.gas;
    bc::Address from = vm::toNativeAddress(msg.sender);
    bc::Address to = vm::toNativeAddress(msg.destination);

}


evmc_tx_context Core::get_tx_context() const noexcept
{
    LOG_DEBUG << "Core::get_tx_context";

    evmc_tx_context ret;
    std::fill(std::begin(ret.tx_gas_price.bytes), std::end(ret.tx_gas_price.bytes), 0);
    ret.block_number = 228;
    ret.block_timestamp = base::Time::now().getSecondsSinceEpochBeginning();
    ret.block_gas_limit = 228;
    std::fill(std::begin(ret.block_difficulty.bytes), std::end(ret.block_difficulty.bytes), 0);
    ret.block_difficulty.bytes[2] = 0x28;

    return ret;
}


evmc::bytes32 Core::get_block_hash(int64_t block_number) const noexcept
{
    LOG_DEBUG << "Core::get_block_hash";
    auto hash = *_blockchain.findBlockHashByDepth(block_number);
    return vm::toEvmcBytes32(hash.getBytes());
}


void Core::emit_log(const evmc::address& addr, const uint8_t* data, size_t data_size, const evmc::bytes32 topics[],
    size_t num_topics) noexcept
{
    LOG_DEBUG << "Core::emit_log";
    LOG_WARNING << "emit_log is denied. For more information, see docs";
}

//===========================


} // namespace lk
