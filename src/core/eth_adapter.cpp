#include "eth_adapter.hpp"

#include "lk/core.hpp"

#include "vm/tools.hpp"
#include "vm/vm.hpp"

namespace lk
{
class EthAdapter::EthHost : public evmc::Host
{
  public:
    EthHost(lk::Core& core, lk::AccountManager& account_manager, lk::CodeManager& code_manager)
      : _core{ core }
      , _account_manager{ account_manager }
      , _code_manager{ code_manager }
    {}


    bool account_exists(const evmc::address& addr) const noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::account_exists";
        auto address = vm::toNativeAddress(addr);
        return _account_manager.hasAccount(address);
    }


    evmc::bytes32 get_storage(const evmc::address& addr, const evmc::bytes32& ethKey) const noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::get_storage";
        try {
            auto address = vm::toNativeAddress(addr);
            base::Bytes key(ethKey.bytes, 32);
            auto storage_value = _account_manager.getAccount(address).getStorageValue(base::Sha256(key)).data;
            return vm::toEvmcBytes32(storage_value);
        }
        catch (...) { // cannot pass exceptions since noexcept
            return {};
        }
    }


    evmc_storage_status set_storage(const evmc::address& addr,
                                    const evmc::bytes32& ekey,
                                    const evmc::bytes32& evalue) noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::set_storage";
        static const base::Bytes NULL_VALUE(32);
        auto address = vm::toNativeAddress(addr);
        auto key = base::Sha256(base::Bytes(ekey.bytes, 32));
        base::Bytes new_value(evalue.bytes, 32);

        auto& account_state = _account_manager.getAccount(address);

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


    evmc::uint256be get_balance(const evmc::address& addr) const noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::get_balance";
        auto address = vm::toNativeAddress(addr);
        auto balance = _account_manager.getBalance(address);
        evmc::uint256be ret;
        std::fill(std::begin(ret.bytes), std::end(ret.bytes), 0);
        for (int i = sizeof(balance) * 8; i >= 0; --i) {
            ret.bytes[i] = balance & 0xFF;
            balance >>= 8;
        }
        return ret;
    }


    size_t get_code_size(const evmc::address& addr) const noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::get_code_size";
        auto address = vm::toNativeAddress(addr);
        auto account_code_hash = _account_manager.getAccount(address).getCodeHash();
        if (auto code = _code_manager.getCode(account_code_hash); !code) {
            ASSERT(false);
            return 0;
        }
        else {
            return code->get().size();
        }
    }


    evmc::bytes32 get_code_hash(const evmc::address& addr) const noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::get_code_hash";
        auto address = vm::toNativeAddress(addr);
        auto account_code_hash = _account_manager.getAccount(address).getCodeHash();
        return vm::toEvmcBytes32(account_code_hash.getBytes());
    }


    size_t copy_code(const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const
      noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::copy_code";
        auto address = vm::toNativeAddress(addr);
        auto account_code_hash = _account_manager.getAccount(address).getCodeHash();
        if (auto code_opt = _code_manager.getCode(account_code_hash); !code_opt) {
            ASSERT(false);
            return 0;
        }
        else {
            const auto& code = code_opt->get();
            std::size_t bytes_to_copy = std::min(buffer_size, code.size() - code_offset);
            std::copy_n(code.getData() + code_offset, bytes_to_copy, buffer_data);
            return bytes_to_copy;
        }
    }


    void selfdestruct(const evmc::address& eaddr, const evmc::address& ebeneficiary) noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::selfdestruct";
        auto address = vm::toNativeAddress(eaddr);
        auto account = _account_manager.getAccount(address);
        auto beneficiary_address = vm::toNativeAddress(ebeneficiary);
        auto beneficiary_account = _account_manager.getAccount(beneficiary_address);

        _account_manager.tryTransferMoney(address, beneficiary_address, account.getBalance());
        _account_manager.deleteAccount(address);
    }


    evmc::result call(const evmc_message& msg) noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::call";

        lk::TransactionBuilder txb;

        txb.setType(lk::Transaction::Type::MESSAGE_CALL);

        lk::Balance fee = msg.gas;
        txb.setFee(fee);

        lk::Address from = vm::toNativeAddress(msg.sender);
        txb.setFrom(std::move(from));

        lk::Address to = vm::toNativeAddress(msg.destination);
        txb.setTo(std::move(to));

        lk::Balance amount = vm::toBalance(msg.value);
        txb.setAmount(amount);

        base::Bytes data(msg.input_data, msg.input_size);
        txb.setData(data);

        auto tx = std::move(txb).build();
        auto result = _core.doMessageCall(tx, *_associated_block);
        return result.getResult();

        // return result;
        // return evmc::make_result(evmc::result::)
    }


    evmc_tx_context get_tx_context() const noexcept override
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


    evmc::bytes32 get_block_hash(int64_t block_number) const noexcept override
    {
        ASSERT(_associated_tx);
        ASSERT(_associated_block);

        LOG_DEBUG << "Core::get_block_hash";
        auto hash = _core.findBlockHash(block_number);
        return vm::toEvmcBytes32(hash->getBytes());
    }


    void emit_log(const evmc::address&, const uint8_t*, size_t, const evmc::bytes32[], size_t) noexcept
    {
        LOG_DEBUG << "Core::emit_log";
        LOG_WARNING << "emit_log is denied. For more information, see docs";
    }

    //===================================
    void setContext(const lk::Transaction* associated_transaction, const lk::Block* associated_block)
    {
        ASSERT(associated_transaction);
        ASSERT(associated_block);

        _associated_tx = associated_transaction;
        _associated_block = associated_block;
    }
    //===================================
  private:
    const lk::Transaction* _associated_tx{ nullptr };
    const lk::Block* _associated_block{ nullptr };
    lk::Core& _core;
    lk::AccountManager& _account_manager;
    lk::CodeManager& _code_manager;
};


EthAdapter::EthAdapter(Core& core, AccountManager& account_manager, CodeManager& code_manager)
  : _eth_host{ std::make_unique<EthHost>(core, account_manager, code_manager) }
  , _vm{ vm::Vm::load(*_eth_host.get()) }
  , _core{ core }
  , _account_manager{ account_manager }
  , _code_manager{ code_manager }
{}


EthAdapter::~EthAdapter() = default;


std::tuple<lk::Address, base::Bytes, lk::Balance> EthAdapter::createContract(const lk::Address& contract_address,
                                                                             const lk::Transaction& associated_tx,
                                                                             const lk::Block& associated_block)
{
    std::lock_guard lk(_execution_mutex);

    base::SerializationIArchive ia(associated_tx.getData());
    auto contract_data = ia.deserialize<lk::ContractInitData>();

    vm::SmartContract contract(contract_data.getCode());
    auto message = contract.createInitMessage(associated_tx.getFee(),
                                              associated_tx.getFrom(),
                                              contract_address,
                                              associated_tx.getAmount(),
                                              contract_data.getInit());

    _eth_host->setContext(&associated_tx, &associated_block);
    if (auto result = _vm.execute(message); result.ok()) {
        // return {contract_address, result.toOutputData()}; -- toOutputData returns the bytecode of contract here
        return { contract_address, {}, static_cast<lk::Balance>(result.gasLeft()) };
    }
    else {
        RAISE_ERROR(base::Error, "contract creation failed during execution");
    }
}


vm::ExecutionResult EthAdapter::call(const lk::Transaction& associated_tx, const lk::Block& associated_block)
{
    std::lock_guard lk(_execution_mutex);

    auto code_hash = _account_manager.getAccount(associated_tx.getTo()).getCodeHash();

    if (auto code_opt = _code_manager.getCode(code_hash); !code_opt) {
        RAISE_ERROR(base::Error, "cannot find code by hash");
    }
    else {
        vm::SmartContract contract(*code_opt);
        auto message = contract.createMessage(associated_tx.getFee(),
                                              associated_tx.getFrom(),
                                              associated_tx.getTo(),
                                              associated_tx.getAmount(),
                                              associated_tx.getData());

        _eth_host->setContext(&associated_tx, &associated_block);
        return _vm.execute(message);
    }
}


} // namespace core