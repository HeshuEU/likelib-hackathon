#pragma once

#include "base/crypto.hpp"
#include "base/property_tree.hpp"
#include "base/utility.hpp"
#include "core/block.hpp"
#include "core/blockchain.hpp"
#include "core/host.hpp"
#include "core/managers.hpp"

#include "vm/vm.hpp"

#include <shared_mutex>

namespace lk
{

class EthHost;

class Core
{
    friend EthHost;

  public:
    //==================
    Core(const base::PropertyTree& config, const base::KeyVault& vault);

    /**
     *  @brief Stops network, does cleaning and flushing.
     *
     *  @threadsafe
     */
    ~Core() = default;
    //==================
    /**
     *  @brief Loads blockchain from disk and runs networking.
     *
     *  @async
     *  @threadsafe
     */
    void run();
    //==================
    lk::AccountInfo getAccountInfo(const lk::Address& address) const;
    //==================
    void addPendingTransaction(const lk::Transaction& tx);
    //==================
    std::optional<TransactionStatus> getTransactionOutput(const base::Sha256& tx_hash);
    void addTransactionOutput(const base::Sha256& tx, const TransactionStatus& status);
    //==================
    Blockchain::AdditionResult tryAddBlock(const ImmutableBlock& b);
    Blockchain::AdditionResult tryAddMinedBlock(const ImmutableBlock& b);
    //==================
    std::optional<ImmutableBlock> findBlock(const base::Sha256& hash) const;
    std::optional<base::Sha256> findBlockHash(const lk::BlockDepth& depth) const;
    std::optional<lk::Transaction> findTransaction(const base::Sha256& hash) const;
    ImmutableBlock getTopBlock() const;
    base::Sha256 getTopBlockHash() const;
    //==================
    std::pair<MutableBlock, lk::Complexity> getMiningData() const;
    //==================
    const lk::Address& getThisNodeAddress() const noexcept;
    //==================
  private:
    //==================
    const base::PropertyTree& _config;
    const base::KeyVault& _vault;
    const lk::Address _this_node_address;
    //==================
    base::Observable<const lk::ImmutableBlock&> _event_block_added;
    base::Observable<const lk::ImmutableBlock&> _event_block_mined;
    base::Observable<const lk::Transaction&> _event_new_pending_transaction;
    base::Observable<base::Sha256> _event_transaction_status_update;
    base::Observable<lk::Address> _event_account_update;
    //==================
    StateManager _state_manager;

    mutable std::shared_mutex _blockchain_mutex;
    PersistentBlockchain _blockchain;

    Blockchain::AdditionResult _tryAddBlock(const ImmutableBlock& b);

    lk::Host _host;
    //==================
    evmc::VM _vm;
    //==================
    lk::TransactionsSet _pending_transactions;
    mutable std::shared_mutex _pending_transactions_mutex;
    //================
    std::unordered_map<base::Sha256, TransactionStatus> _tx_outputs;
    mutable std::shared_mutex _tx_outputs_mutex;
    //==================
    static const ImmutableBlock& getGenesisBlock();
    void applyBlockTransactions(const ImmutableBlock& block);
    //==================
    // Only called from tryAddBlock -- just a helper function, not thread safe
    bool checkBlockTransactions(const ImmutableBlock& block) const;
    //==================
    void tryPerformTransaction(const lk::Transaction& tx, const ImmutableBlock& block_where_tx);
    //==================
    void on_account_updated(lk::Address address);
    //==================
    evmc::result callInitContractVm(Commit& current_commit,
                                    const ImmutableBlock& associated_block,
                                    const lk::Transaction& tx,
                                    const lk::Address& contract_address,
                                    const base::Bytes& code);
    evmc::result callContractVm(Commit& current_commit,
                                const ImmutableBlock& associated_block,
                                const lk::Transaction& tx,
                                const base::Bytes& code,
                                const base::Bytes& message_data);
    evmc::result callVm(Commit& current_commit,
                        const ImmutableBlock& associated_block,
                        const lk::Transaction& associated_tx,
                        const evmc_message& message,
                        const base::Bytes& code);

  public:
    //==================
    // notifies if new blocks are added: genesis and blocks, that are stored in DB, are not handled by this
    void subscribeToBlockAddition(decltype(_event_block_mined)::CallbackType callback);

    // notifies if a block was mined by this node and it was added to blockchain
    void subscribeToBlockMining(decltype(_event_block_mined)::CallbackType callback);

    // notifies if some transaction was added to set of pending
    void subscribeToNewPendingTransaction(decltype(_event_new_pending_transaction)::CallbackType callback);

    // notifies if any transaction status was updated
    void subscribeToAnyTransactionStatusUpdate(decltype(_event_transaction_status_update)::CallbackType callback);

    // notifies if any account was updated
    void subscribeToAnyAccountUpdate(decltype(_event_account_update)::CallbackType callback);
    //==================
};


class EthHost : public evmc::Host
{
  public:
    EthHost(lk::Core& core,
            lk::Commit& current_commit,
            const ImmutableBlock& associated_block,
            const lk::Transaction& associated_tx);

    bool account_exists(const evmc::address& addr) const noexcept override;

    evmc::bytes32 get_storage(const evmc::address& addr, const evmc::bytes32& ethKey) const noexcept override;

    evmc_storage_status set_storage(const evmc::address& addr,
                                    const evmc::bytes32& ekey,
                                    const evmc::bytes32& evalue) noexcept override;

    evmc::uint256be get_balance(const evmc::address& addr) const noexcept override;

    size_t get_code_size(const evmc::address& addr) const noexcept override;

    evmc::bytes32 get_code_hash(const evmc::address& addr) const noexcept override;

    size_t copy_code(const evmc::address& addr,
                     size_t code_offset,
                     uint8_t* buffer_data,
                     size_t buffer_size) const noexcept override;

    void selfdestruct(const evmc::address& eaddr, const evmc::address& ebeneficiary) noexcept override;

    evmc::result call(const evmc_message& msg) noexcept override;

    evmc_tx_context get_tx_context() const noexcept override;

    evmc::bytes32 get_block_hash(int64_t block_number) const noexcept override;

    void emit_log(const evmc::address&, const uint8_t*, size_t, const evmc::bytes32[], size_t) noexcept override;

  private:
    Core& _core;
    Commit& _current_commit;
    const ImmutableBlock& _associated_block;
    const Transaction& _associated_tx;
};

} // namespace core
