#pragma once

#include "base/property_tree.hpp"
#include "base/crypto.hpp"
#include "base/utility.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "lk/account_manager.hpp"
#include "lk/protocol.hpp"
#include "net/host.hpp"
#include "vm/vm.hpp"

#include <shared_mutex>

namespace lk
{

class Core : public evmc::Host
{
  public:
    //==================
    Core(const base::PropertyTree& config, const base::KeyVault& vault);

    /**
     *  @brief Stops network, does cleaning and flushing.
     *
     *  @threadsafe
     */
    ~Core() override = default;
    //==================
    /**
     *  @brief Loads blockchain from disk and runs networking.
     *
     *  @async
     *  @threadsafe
     */
    void run();
    //==================
    bc::Balance getBalance(const bc::Address& address) const;
    //==================
    bool addPendingTransaction(const bc::Transaction& tx);
    //==================
    bool tryAddBlock(const bc::Block& b);
    std::optional<bc::Block> findBlock(const base::Sha256& hash) const;
    const bc::Block& getTopBlock() const;
    //==================
    bc::Block getBlockTemplate() const;
    //==================
    base::Bytes getThisNodeAddress() const;
    //==================
    bool account_exists(const evmc::address& addr) const noexcept override;

    evmc::bytes32 get_storage(const evmc::address& addr, const evmc::bytes32& key) const noexcept override;

    evmc_storage_status set_storage(
        const evmc::address& addr, const evmc::bytes32& key, const evmc::bytes32& value) noexcept override;

    evmc::uint256be get_balance(const evmc::address& addr) const noexcept override;

    size_t get_code_size(const evmc::address& addr) const noexcept override;

    evmc::bytes32 get_code_hash(const evmc::address& addr) const noexcept override;

    size_t copy_code(const evmc::address& addr, size_t code_offset, uint8_t* buffer_data, size_t buffer_size) const
        noexcept override;

    void selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept override;

    evmc::result call(const evmc_message& msg) noexcept override;

    evmc_tx_context get_tx_context() const noexcept override;

    evmc::bytes32 get_block_hash(int64_t block_number) const noexcept override;

    void emit_log(const evmc::address& addr, const uint8_t* data, size_t data_size, const evmc::bytes32 topics[],
        size_t num_topics) noexcept override;
    //==================
  private:
    //==================
    const base::PropertyTree& _config;
    const base::KeyVault& _vault;
    //==================
    base::Observable<const bc::Block&> _event_block_added;
    base::Observable<const bc::Transaction&> _event_new_pending_transaction;
    //==================
    bool _is_account_manager_updated{false};
    AccountManager _account_manager;
    bc::Blockchain _blockchain;
    lk::Network _network;
    //==================
    bc::TransactionsSet _pending_transactions;
    mutable std::shared_mutex _pending_transactions_mutex;
    //==================
    static const bc::Block& getGenesisBlock();
    void applyBlockTransactions(const bc::Block& block);
    //==================
    bool checkBlock(const bc::Block& block) const;
    bool checkTransaction(const bc::Transaction& tx) const;
    //==================
    bool tryPerformTransaction(const bc::Transaction& tx);
    bc::Address doContractCreation(const bc::Transaction& tx);
    vm::ExecutionResult doMessageCall(const bc::Transaction& tx);
    //==================
  public:
    //==================
    // notifies if new blocks are added: genesis and blocks, that are stored in DB, are not handled by this
    void subscribeToBlockAddition(decltype(_event_block_added)::CallbackType callback);

    // notifies if some transaction was added to set of pending
    void subscribeToNewPendingTransaction(decltype(_event_new_pending_transaction)::CallbackType callback);
    //==================
};

} // namespace lk
