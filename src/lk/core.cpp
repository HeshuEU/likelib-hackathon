#include "core.hpp"

#include "base/log.hpp"
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
    _is_account_manager_updated = true;
    _blockchain.load();
}


const bc::Block& Core::getGenesisBlock()
{
    static bc::Block genesis = [] {
        bc::Block ret{0, base::Sha256(base::Bytes(32)), {}};
        bc::Address from{bc::Address::null()};
        bc::Address to{"UTpE8/SckOrfV4Fn/Gi3jmLEOVI="};
        bc::Balance amount{0xFFFFFFFF};
        bc::Balance fee{0};
        auto timestamp = base::Time::fromSecondsSinceEpochBeginning(0);

        ret.addTransaction({from, to, amount, fee, timestamp, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{}});
        return ret;
    }();
    return genesis;
}


void Core::run()
{
    _network.run();
}


bool Core::addPendingTransaction(const bc::Transaction& tx)
{
    if(checkTransaction(tx)) {
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


void Core::addPendingTransactionAndWait(const bc::Transaction& tx)
{
    if(!checkTransaction(tx)) {
        RAISE_ERROR(base::InvalidArgument, "invalid transaction");
    }

    std::condition_variable cv;
    std::mutex mt;
    bool is_tx_mined = false;

    auto id = _event_block_added.subscribe([&cv, &tx, &is_tx_mined](const bc::Block& block) {
        if(block.getTransactions().find(tx)) {
            is_tx_mined = true;
            cv.notify_all();
        }
    });

    addPendingTransaction(tx);

    std::unique_lock lk(mt);
    cv.wait(lk, [&is_tx_mined] {
        return is_tx_mined;
    });
    _event_block_added.unsubscribe(id);
}


base::Bytes Core::getTransactionOutput(const base::Sha256& tx)
{
    std::shared_lock lk(_tx_outputs_mutex);
    if(auto it = _tx_outputs.find(tx); it != _tx_outputs.end()) {
        return it->second;
    }
    else {
        return {};
    }
}


bool Core::tryAddBlock(const bc::Block& b)
{
    if(checkBlock(b) && _blockchain.tryAddBlock(b)) {
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


std::optional<bc::Block> Core::findBlock(const base::Sha256& hash) const
{
    return _blockchain.findBlock(hash);
}


std::optional<base::Sha256> Core::findBlockHash(const bc::BlockDepth& depth) const
{
    return _blockchain.findBlockHashByDepth(depth);
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
        LOG_DEBUG << "Failed signature verification";
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


void Core::applyBlockTransactions(const bc::Block& block)
{
    if(!_is_account_manager_updated) {
        _account_manager.updateFromGenesis(block);
        _is_account_manager_updated = true;
    }
    else {
        for(const auto& tx: block.getTransactions()) {
            tryPerformTransaction(tx);
        }
    }
}


bool Core::tryPerformTransaction(const bc::Transaction& tx)
{
    auto hash = base::Sha256::compute(base::toBytes(tx)).getBytes();
    if(tx.getType() == bc::Transaction::Type::CONTRACT_CREATION) {
        try {
            auto [address, result] = doContractCreation(tx);
            LOG_DEBUG << "Contract created at " << address << " with output = " << result.toHex();
            base::SerializationOArchive oa;
            oa.serialize(address);
            oa.serialize(result);
            {
                std::unique_lock lk(_tx_outputs_mutex);
                _tx_outputs[hash] = std::move(oa).getBytes();
            }
        }
        catch(const base::Error&) {
            return false;
        }
        return true;
    }
    else {
        auto result = doMessageCall(tx);
        LOG_DEBUG << "Message call result: " << result.toHex();
        {
            std::unique_lock lk(_tx_outputs_mutex);
            _tx_outputs[hash] = base::toBytes(result);
        }
        return true;
    }
}


std::pair<bc::Address, base::Bytes> Core::doContractCreation(const bc::Transaction& tx)
{
    base::SerializationIArchive ia(tx.getData());
    auto contract_data = ia.deserialize<bc::ContractInitData>();

    vm::SmartContract contract(contract_data.getCode());
    auto vm = vm::Vm::load(*this);
    bc::Address contract_address = _account_manager.newContract(tx.getFrom(), contract_data.getCode());
    LOG_DEBUG << "Deploying smart contract at address " << contract_address;
    if(tx.getAmount() != 0) {
        if(!_account_manager.tryTransferMoney(tx.getFrom(), tx.getTo(), tx.getAmount())) {
            RAISE_ERROR(base::Error, "cannot transfer money");
        }
    }
    auto message = contract.createInitMessage(
        tx.getFee(), tx.getFrom(), contract_address, tx.getAmount(), contract_data.getInit());
    if(auto result = vm.execute(message); result.ok()) {
        return {contract_address, result.toOutputData()};
    }
    else {
        RAISE_ERROR(base::Error, "invalid result");
    }
}


base::Bytes Core::doMessageCall(const bc::Transaction& tx)
{
    auto code_hash = _account_manager.getAccount(tx.getTo()).getCodeHash();

    if(!_account_manager.tryTransferMoney(tx.getFrom(), tx.getTo(), tx.getAmount())) {
        RAISE_ERROR(base::Error, "cannot transfer money");
    }

    if(code_hash != base::Sha256::null()) {
        // if we're here -- do a call to a contract
        if(auto c = _code_manager.getCode(code_hash); !c) {
            RAISE_ERROR(base::Error, "cannot find code by hash");
        }
        else {
            const auto& code = *c;
            EthAdapter runner(code, _account_manager, _code_manager);
            runner.messageCall(tx.getFee(), tx.getFrom(), tx.getTo(), tx.getAmount(), tx.getData());

            auto message = contract.createMessage();
            auto ret = vm.execute(message);
            return ret.toOutputData();
        }
    }
    else {
        _account_manager.tryTransferMoney(tx.getFrom(), tx.getTo(), tx.getAmount());
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
