#pragma once

#include "block.hpp"
#include "blockchain.hpp"
#include "host.hpp"
#include "managers.hpp"
#include "protocol.hpp"

#include <vm/vm.hpp>

#include "base/crypto.hpp"
#include "base/property_tree.hpp"
#include "base/utility.hpp"

#include <shared_mutex>

namespace lk
{

class EthHost : public evmc::Host
{
  public:
    EthHost(lk::Core& core, lk::StateManager& state_manager);

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

    void emit_log(const evmc::address&, const uint8_t*, size_t, const evmc::bytes32[], size_t) noexcept;

    void setContext(const lk::Transaction* associated_transaction, const lk::Block* associated_block);

  private:
    const lk::Transaction* _associated_tx{ nullptr };
    const lk::Block* _associated_block{ nullptr };
    //==================
    lk::Core& _core;
    lk::StateManager& _state_manager;
};


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
    bool addPendingTransaction(const lk::Transaction& tx);
    void addPendingTransactionAndWait(const lk::Transaction& tx);
    TransactionStatus getTransactionOutput(const base::Sha256& tx_hash);
    //==================
    bool tryAddBlock(const lk::Block& b);
    std::optional<lk::Block> findBlock(const base::Sha256& hash) const;
    std::optional<base::Sha256> findBlockHash(const lk::BlockDepth& depth) const;
    std::optional<lk::Transaction> findTransaction(const base::Sha256& hash) const;
    const lk::Block& getTopBlock() const;
    //==================
    lk::Block getBlockTemplate() const;
    //==================
    const lk::Address& getThisNodeAddress() const noexcept;
    //==================
    base::Bytes callViewMethod(const lk::Address& from,
                               const lk::Address& contract_address,
                               const base::Bytes& message);

  private:
    //==================
    const base::PropertyTree& _config;
    const base::KeyVault& _vault;
    const lk::Address _this_node_address;
    //==================
    base::Observable<const lk::Block&> _event_block_added;
    base::Observable<const lk::Transaction&> _event_new_pending_transaction;
    //==================
    bool _is_account_manager_updated{ false };
    StateManager _state_manager;
    lk::Blockchain _blockchain;
    lk::Host _host;
    //==================
    std::unique_ptr<EthHost> _eth_host;
    evmc::VM _vm;
    //==================
    lk::TransactionsSet _pending_transactions;
    mutable std::shared_mutex _pending_transactions_mutex;
    //==================
    static const lk::Block& getGenesisBlock();
    void applyBlockTransactions(const lk::Block& block);
    //==================
    bool checkBlock(const lk::Block& block) const;
    bool checkTransaction(const lk::Transaction& tx) const;
    //==================
    bool tryPerformTransaction(const lk::Transaction& tx, const lk::Block& block_where_tx);
    //==================
    evmc::result callInitContractVm(const lk::Transaction& tx,
                                    const lk::Address& contract_address,
                                    const base::Bytes& code);

    evmc::result callContractVm(const lk::Transaction& tx, const base::Bytes& code, const base::Bytes& message_data);

    evmc::result callContractAtViewModeVm(const lk::Address& sender_address,
                                          const lk::Address& contract_address,
                                          const base::Bytes& code,
                                          const base::Bytes& message_data);

  public:
    //==================
    // notifies if new blocks are added: genesis and blocks, that are stored in DB, are not handled by this
    void subscribeToBlockAddition(decltype(_event_block_added)::CallbackType callback);

    // notifies if some transaction was added to set of pending
    void subscribeToNewPendingTransaction(decltype(_event_new_pending_transaction)::CallbackType callback);
    //==================
};

} // namespace core
