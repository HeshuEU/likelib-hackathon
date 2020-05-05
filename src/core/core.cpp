#include "core.hpp"

#include <vm/error.hpp>
#include <vm/tools.hpp>

#include <base/log.hpp>

#include <algorithm>


namespace lk
{


EthHost::EthHost(lk::Core& core, lk::StateManager& state_manager)
  : _core{ core }
  , _state_manager{ state_manager }
{}


bool EthHost::account_exists(const evmc::address& addr) const noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::account_exists";
    auto address = vm::toNativeAddress(addr);
    return _state_manager.hasAccount(address);
}


evmc::bytes32 EthHost::get_storage(const evmc::address& addr, const evmc::bytes32& ethKey) const noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::get_storage";
    try {
        auto address = vm::toNativeAddress(addr);
        base::Bytes key(ethKey.bytes, 32);
        auto storage_value = _state_manager.getAccount(address).getStorageValue(base::Sha256(key)).data;
        return vm::toEvmcBytes32(storage_value);
    }
    catch (...) { // cannot pass exceptions since noexcept
        return {};
    }
}


evmc_storage_status EthHost::set_storage(const evmc::address& addr,
                                         const evmc::bytes32& ekey,
                                         const evmc::bytes32& evalue) noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::set_storage";
    static const base::Bytes NULL_VALUE(32);
    auto address = vm::toNativeAddress(addr);
    auto key = base::Sha256(base::Bytes(ekey.bytes, 32));
    base::Bytes new_value(evalue.bytes, 32);

    auto& account_state = _state_manager.getAccount(address);

    if (!account_state.checkStorageValue(key)) {
        if (new_value == NULL_VALUE) {
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
        if (old_value == new_value) {
            return evmc_storage_status::EVMC_STORAGE_UNCHANGED;
        }
        else if (new_value == NULL_VALUE) {
            return evmc_storage_status::EVMC_STORAGE_DELETED;
        }
        else {
            return evmc_storage_status::EVMC_STORAGE_MODIFIED;
        }
    }
}


evmc::uint256be EthHost::get_balance(const evmc::address& addr) const noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::get_balance";
    auto address = vm::toNativeAddress(addr);
    auto balance = _state_manager.getBalance(address);
    return vm::toEvmcUint256(balance);
}


size_t EthHost::get_code_size(const evmc::address& addr) const noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::get_code_size";
    auto address = vm::toNativeAddress(addr);
    if (auto code = _state_manager.getAccount(address).getRuntimeCode(); code.isEmpty()) {
        ASSERT(false);
        return 0;
    }
    else {
        return code.size();
    }
}


evmc::bytes32 EthHost::get_code_hash(const evmc::address& addr) const noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::get_code_hash";
    auto address = vm::toNativeAddress(addr);
    auto account_code_hash = _state_manager.getAccount(address).getCodeHash();
    return vm::toEvmcBytes32(account_code_hash.getBytes());
}


size_t EthHost::copy_code(const evmc::address& addr,
                          size_t code_offset,
                          uint8_t* buffer_data,
                          size_t buffer_size) const noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::copy_code";
    auto address = vm::toNativeAddress(addr);
    if (auto code = _state_manager.getAccount(address).getRuntimeCode(); code.isEmpty()) {
        ASSERT(false);
        return 0;
    }
    else {
        std::size_t bytes_to_copy = std::min(buffer_size, code.size() - code_offset);
        std::copy_n(code.getData() + code_offset, bytes_to_copy, buffer_data);
        return bytes_to_copy;
    }
}


void EthHost::selfdestruct(const evmc::address& eaddr, const evmc::address& ebeneficiary) noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::selfdestruct";
    auto address = vm::toNativeAddress(eaddr);
    auto account = _state_manager.getAccount(address);
    auto beneficiary_address = vm::toNativeAddress(ebeneficiary);
    auto beneficiary_account = _state_manager.getAccount(beneficiary_address);

    _state_manager.tryTransferMoney(address, beneficiary_address, account.getBalance());
    _state_manager.deleteAccount(address);
}


evmc::result EthHost::call(const evmc_message& msg) noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::call";

    lk::Address to = vm::toNativeAddress(msg.destination);
    if (_state_manager.getAccount(to).getType() == lk::AccountType::CONTRACT) {
        auto code = _state_manager.getAccount(to).getRuntimeCode();
        return _core._vm.execute(*this, evmc_revision::EVMC_ISTANBUL, msg, code.getData(), code.size());
    }
    else {
        lk::Address from = vm::toNativeAddress(msg.sender);
        _state_manager.tryTransferMoney(from, to, vm::toBalance(msg.value));
        evmc::result result{ evmc_status_code::EVMC_SUCCESS, msg.gas, nullptr, 0 };
        return result;
    }
}


evmc_tx_context EthHost::get_tx_context() const noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::get_tx_context";

    evmc_tx_context ret;
    std::fill(std::begin(ret.tx_gas_price.bytes), std::end(ret.tx_gas_price.bytes), 0);
    ret.tx_origin = vm::toEthAddress(_associated_tx->getFrom());
    ret.block_number = _associated_block->getDepth();
    ret.block_timestamp = _associated_block->getTimestamp().getSecondsSinceEpoch();
    ret.block_coinbase = vm::toEthAddress(_associated_block->getCoinbase());
    // ret.block_gas_limit
    std::fill(std::begin(ret.block_difficulty.bytes), std::end(ret.block_difficulty.bytes), 0);
    ret.block_difficulty.bytes[2] = 0x28;

    return ret;
}


evmc::bytes32 EthHost::get_block_hash(int64_t block_number) const noexcept
{
    ASSERT(_associated_tx);
    ASSERT(_associated_block);

    LOG_DEBUG << "Core::get_block_hash";
    auto hash = _core.findBlockHash(block_number);
    return vm::toEvmcBytes32(hash->getBytes());
}


void EthHost::emit_log(const evmc::address&, const uint8_t*, size_t, const evmc::bytes32[], size_t) noexcept
{
    LOG_DEBUG << "Core::emit_log";
    LOG_WARNING << "emit_log is denied. For more information, see docs";
}

//===================================
void EthHost::setContext(const lk::Transaction* associated_transaction, const lk::Block* associated_block)
{
    ASSERT(associated_transaction);
    ASSERT(associated_block);

    _associated_tx = associated_transaction;
    _associated_block = associated_block;
}


Core::Core(const base::PropertyTree& config, const base::KeyVault& key_vault)
  : _config{ config }
  , _vault{ key_vault }
  , _this_node_address{ _vault.getPublicKey() }
  , _blockchain{ _config }
  , _host{ _config, 0xFFFF, *this }
  , _eth_host{ std::make_unique<EthHost>(*this, _state_manager) }
  , _vm{ std::move(vm::load()) }
{
    [[maybe_unused]] bool result = _blockchain.tryAddBlock(getGenesisBlock());
    ASSERT(result);
    _state_manager.updateFromGenesis(getGenesisBlock());
    _is_account_manager_updated = true;

    _blockchain.load();
    for (lk::BlockDepth d = 1; d <= _blockchain.getTopBlock().getDepth(); ++d) {
        auto block = *_blockchain.findBlock(*_blockchain.findBlockHashByDepth(d));
        for (const auto& tx : block.getTransactions()) {
            tryPerformTransaction(tx, block);
        }
    }

    subscribeToBlockAddition([this](const lk::Block& block) { _host.broadcast(block); });
    subscribeToNewPendingTransaction([this](const lk::Transaction& tx) { _host.broadcast(tx); });
}


const lk::Block& Core::getGenesisBlock()
{
    static lk::Block genesis = [] {
        auto timestamp = base::Time(1583789617);

        lk::Block ret{ 0, base::Sha256(base::Bytes(32)), timestamp, lk::Address::null(), {} };
        lk::Address from{ lk::Address::null() };
        lk::Address to{ "28dpzpURpyqqLoWrEhnHrajndeCq" };
        lk::Balance amount{ 0xFFFFFFFF };
        lk::Balance fee{ 0 };

        ret.addTransaction({ from, to, amount, fee, timestamp, base::Bytes{} });
        return ret;
    }();
    return genesis;
}


void Core::run()
{
    _host.run();
}


bool Core::addPendingTransaction(const lk::Transaction& tx)
{
    if (checkTransaction(tx)) {
        {
            LOG_DEBUG << "Adding tx to pending";
            std::unique_lock lk(_pending_transactions_mutex);
            _pending_transactions.add(tx);
        }
        _event_new_pending_transaction.notify(tx);
        return true;
    }
    return false;
}


void Core::addPendingTransactionAndWait(const lk::Transaction& tx)
{
    if (!checkTransaction(tx)) {
        RAISE_ERROR(base::InvalidArgument, "invalid transaction");
    }

    std::condition_variable cv;
    std::mutex mt;
    bool is_tx_mined = false;

    auto id = _event_block_added.subscribe([&cv, &tx, &is_tx_mined](const lk::Block& block) {
        if (block.getTransactions().find(tx)) {
            is_tx_mined = true;
            cv.notify_all();
        }
    });

    addPendingTransaction(tx);

    std::unique_lock lk(mt);
    cv.wait(lk, [&is_tx_mined] { return is_tx_mined; });
    _event_block_added.unsubscribe(id);
}


TransactionStatus Core::getTransactionOutput(const base::Sha256& tx)
{
    return _state_manager.getTransactionOutput(tx);
}


bool Core::tryAddBlock(const lk::Block& b)
{
    if (checkBlock(b) && _blockchain.tryAddBlock(b)) {
        {
            std::shared_lock lk(_pending_transactions_mutex);
            _pending_transactions.remove(b.getTransactions());
        }
        LOG_DEBUG << "Applying transactions from block #" << b.getDepth();
        applyBlockTransactions(b);
        _event_block_added.notify(b);
        return true;
    }
    else {
        return false;
    }
}


std::optional<lk::Block> Core::findBlock(const base::Sha256& hash) const
{
    return _blockchain.findBlock(hash);
}


std::optional<base::Sha256> Core::findBlockHash(const lk::BlockDepth& depth) const
{
    return _blockchain.findBlockHashByDepth(depth);
}


std::optional<lk::Transaction> Core::findTransaction(const base::Sha256& hash) const
{
    return _blockchain.findTransaction(hash);
}

bool Core::checkBlock(const lk::Block& b) const
{
    if (_blockchain.findBlock(base::Sha256::compute(base::toBytes(b)))) {
        return false;
    }

    // FIXME: this works wrong if two transactions are both valid, but together are not
    for (const auto& tx : b.getTransactions()) {
        if (!_state_manager.checkTransaction(tx)) {
            return false;
        }
    }

    return true;
}


bool Core::checkTransaction(const lk::Transaction& tx) const
{
    if (!tx.checkSign()) {
        LOG_DEBUG << "Failed signature verification";
        return false;
    }

    if (_blockchain.findTransaction(tx.hashOfTransaction())) {
        return false;
    }

    std::map<lk::Address, lk::Balance> current_pending_balance;
    {
        std::shared_lock lk(_pending_transactions_mutex);
        if (_pending_transactions.find(tx)) {
            return false;
        }

        current_pending_balance = lk::calcBalance(_pending_transactions);
    }

    auto pending_from_account_balance = current_pending_balance.find(tx.getFrom());
    if (pending_from_account_balance != current_pending_balance.end()) {
        auto current_from_account_balance = _state_manager.getBalance(tx.getFrom());
        return pending_from_account_balance->second + current_from_account_balance >= tx.getAmount();
    }
    else {
        return _state_manager.checkTransaction(tx);
    }
}


lk::Block Core::getBlockTemplate() const
{
    const auto& top_block = _blockchain.getTopBlock();
    lk::BlockDepth depth = top_block.getDepth() + 1;
    auto prev_hash = base::Sha256::compute(base::toBytes(top_block));
    std::shared_lock lk(_pending_transactions_mutex);
    return lk::Block{ depth, prev_hash, base::Time::now(), getThisNodeAddress(), _pending_transactions };
}

lk::AccountInfo Core::getAccountInfo(const lk::Address& address) const
{
    return _state_manager.getAccount(address).toInfo();
}


const lk::Block& Core::getTopBlock() const
{
    return _blockchain.getTopBlock();
}


const lk::Address& Core::getThisNodeAddress() const noexcept
{
    return _this_node_address;
}


base::Bytes Core::callViewMethod(const lk::Address& from,
                                 const lk::Address& contract_address,
                                 const base::Bytes& message)
{
    auto contract_account = _state_manager.getAccount(contract_address);
    auto contract_abi = contract_account.getAbi();
    if (vm::isView(contract_abi, message)) {
        auto tx = invalidTransaction();
        _eth_host->setContext(&tx, &_blockchain.getTopBlock());
        auto eval_result = callContractAtViewModeVm(from, contract_address, contract_account.getRuntimeCode(), message);
        auto output_data = vm::copy(eval_result.output_data, eval_result.output_size);
        return output_data;
    }
    else {
        RAISE_ERROR(base::InvalidArgument, "not a view method");
    }
}


void Core::applyBlockTransactions(const lk::Block& block)
{
    if (!_is_account_manager_updated) {
        _state_manager.updateFromGenesis(block);
        _is_account_manager_updated = true;
    }
    else {
        for (const auto& tx : block.getTransactions()) {
            tryPerformTransaction(tx, block);
        }
        constexpr lk::Balance EMISSION_VALUE = 1000;
        _state_manager.getAccount(block.getCoinbase()).addBalance(EMISSION_VALUE);
    }
}


bool Core::tryPerformTransaction(const lk::Transaction& tx, const lk::Block& block_where_tx)
{
    auto transaction_hash = tx.hashOfTransaction();

    lk::AccountState from_account_recovery{ _state_manager.getAccount(tx.getFrom()) };

    if (tx.getTo() == lk::Address::null()) {
        try {
            _state_manager.getAccount(tx.getFrom()).subBalance(tx.getFee());

            auto contract_data = base::fromBytes<lk::ContractData>(tx.getData());

            auto contract_data_hash = base::Sha256::compute(contract_data.getMessage());
            lk::Address contract_address = _state_manager.createContractAccount(tx.getFrom(), contract_data_hash);
            LOG_DEBUG << "Deploying smart contract at address " << contract_address;
            if (tx.getAmount() != 0) {
                if (!_state_manager.tryTransferMoney(tx.getFrom(), contract_address, tx.getAmount())) {
                    RAISE_ERROR(base::Error, "cannot transfer money");
                }
            }

            lk::Balance gas_left = 0;
            try {
                _eth_host->setContext(&tx, &block_where_tx);
                auto eval_result = callInitContractVm(tx, contract_address, contract_data.getMessage());
                if (eval_result.status_code != evmc_status_code::EVMC_SUCCESS) {
                    RAISE_ERROR(base::Error, "fail at contract creation");
                }
                auto runtime_code = vm::copy(eval_result.output_data, eval_result.output_size);
                _state_manager.getAccount(contract_address).setRuntimeCode(runtime_code);
                _state_manager.getAccount(contract_address).setAbi(contract_data.getAbi());
                gas_left = eval_result.gas_left;
            }
            catch (const vm::RevertError& e) {
                _state_manager.deleteAccount(contract_address);
                _state_manager.getAccount(tx.getFrom()) = from_account_recovery;
                RAISE_ERROR(base::Error, "fail at contract creation");
            }

            LOG_DEBUG << "Contract created at " << contract_address;

            TransactionStatus status(TransactionStatus::StatusCode::Success,
                                     TransactionStatus::ActionType::ContractCreation,
                                     gas_left,
                                     base::base58Encode(contract_address.getBytes()));

            _state_manager.addTransactionOutput(transaction_hash, status);
            _state_manager.getAccount(block_where_tx.getCoinbase()).addBalance(tx.getFee() - gas_left);
            _state_manager.getAccount(tx.getFrom()).addBalance(gas_left);
        }
        catch (const base::Error&) {
            return false;
        }
        return true;
    }
    else {
        lk::AccountState to_account_recovery{ _state_manager.getAccount(tx.getTo()) };
        try {
            _state_manager.getAccount(tx.getFrom()).subBalance(tx.getFee());
            //            auto result = doMessageCall(tx, block_where_tx);

            if (!_state_manager.tryTransferMoney(tx.getFrom(), tx.getTo(), tx.getAmount())) {
                RAISE_ERROR(base::Error, "cannot transfer money");
            }

            if (_state_manager.getAccount(tx.getTo()).getType() == AccountType::CONTRACT) {
                _eth_host->setContext(&tx, &block_where_tx);
                auto code = _state_manager.getAccount(tx.getTo()).getRuntimeCode();

                auto eval_result = callContractVm(tx, code, tx.getData());
                auto output_data = vm::copy(eval_result.output_data, eval_result.output_size);
                if (eval_result.status_code == evmc_status_code::EVMC_SUCCESS) {
                    LOG_DEBUG << "Message call result: " << base::toHex(output_data);

                    TransactionStatus status(TransactionStatus::StatusCode::Success,
                                             TransactionStatus::ActionType::ContractCall,
                                             eval_result.gas_left,
                                             base::base64Encode(output_data));

                    _state_manager.addTransactionOutput(transaction_hash, status);
                    _state_manager.getAccount(block_where_tx.getCoinbase())
                      .addBalance(tx.getFee() - eval_result.gas_left);
                    _state_manager.getAccount(tx.getFrom()).addBalance(eval_result.gas_left);
                    return true;
                }
                else {
                    RAISE_ERROR(base::Error, "evm has not success execution status");
                }
            }
            else {
                TransactionStatus status(
                  TransactionStatus::StatusCode::Success, TransactionStatus::ActionType::Transfer, 0, {});

                _state_manager.addTransactionOutput(transaction_hash, status);
                return true;
            }
        }
        catch (const base::Error&) {
            _state_manager.getAccount(tx.getFrom()) = from_account_recovery;
            _state_manager.getAccount(tx.getTo()) = to_account_recovery;
            return false;
        }
    }
}


evmc::result Core::callInitContractVm(const lk::Transaction& tx,
                                      const lk::Address& contract_address,
                                      const base::Bytes& code)
{
    evmc_message message;
    message.kind = evmc_call_kind::EVMC_CREATE;
    message.depth = 0;
    message.gas = tx.getFee();
    message.sender = vm::toEthAddress(tx.getFrom());
    message.destination = vm::toEthAddress(contract_address);
    message.value = vm::toEvmcUint256(tx.getAmount());
    message.create2_salt = evmc_bytes32();
    return _vm.execute(*(_eth_host.get()), evmc_revision::EVMC_ISTANBUL, message, code.getData(), code.size());
}


evmc::result Core::callContractVm(const lk::Transaction& tx, const base::Bytes& code, const base::Bytes& message_data)
{
    evmc_message message;
    message.kind = evmc_call_kind::EVMC_CALL;
    message.depth = 0;
    message.gas = tx.getFee();
    message.sender = vm::toEthAddress(tx.getFrom());
    message.destination = vm::toEthAddress(tx.getTo());
    message.value = vm::toEvmcUint256(tx.getAmount());
    message.input_data = message_data.getData();
    message.input_size = message_data.size();
    return _vm.execute(*(_eth_host.get()), evmc_revision::EVMC_ISTANBUL, message, code.getData(), code.size());
}


evmc::result Core::callContractAtViewModeVm(const lk::Address& sender_address,
                                            const lk::Address& contract_address,
                                            const base::Bytes& code,
                                            const base::Bytes& message_data)
{
    evmc_message message;
    message.kind = evmc_call_kind::EVMC_CALL;
    message.flags = evmc_flags::EVMC_STATIC;
    message.depth = 0;
    constexpr lk::Balance VIEW_FREE_MAX_VALUE = 5000;
    message.gas = VIEW_FREE_MAX_VALUE;
    message.sender = vm::toEthAddress(sender_address);
    message.destination = vm::toEthAddress(contract_address);
    message.value = vm::toEvmcUint256(0);
    message.input_data = message_data.getData();
    message.input_size = message_data.size();
    return _vm.execute(*(_eth_host.get()), evmc_revision::EVMC_ISTANBUL, message, code.getData(), code.size());
}


void Core::subscribeToBlockAddition(decltype(Core::_event_block_added)::CallbackType callback)
{
    _event_block_added.subscribe(std::move(callback));
}


void Core::subscribeToNewPendingTransaction(decltype(Core::_event_new_pending_transaction)::CallbackType callback)
{
    _event_new_pending_transaction.subscribe(std::move(callback));
}


} // namespace core
