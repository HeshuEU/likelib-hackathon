#pragma once

#include "base/crypto.hpp"
#include "base/property_tree.hpp"
#include "base/utility.hpp"
#include "core/block.hpp"
#include "core/blockchain.hpp"
#include "core/eth_adapter.hpp"
#include "core/host.hpp"
#include "core/managers.hpp"

#include <shared_mutex>

namespace lk
{

class EthAdapter;

class Core
{
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
    lk::Balance getBalance(const lk::Address& address) const;
    //==================
    bool addPendingTransaction(const lk::Transaction& tx);
    void addPendingTransactionAndWait(const lk::Transaction& tx);
    base::Bytes getTransactionOutput(const base::Sha256& tx_hash);
    //==================
    bool tryAddBlock(const lk::Block& b);
    std::optional<lk::Block> findBlock(const base::Sha256& hash) const;
    std::optional<base::Sha256> findBlockHash(const lk::BlockDepth& depth) const;
    const lk::Block& getTopBlock() const;
    //==================
    lk::Block getBlockTemplate() const;
    //==================
    const lk::Address& getThisNodeAddress() const noexcept;
    //==================
  private:
    //==================
    friend class lk::EthAdapter;
    //==================
    const base::PropertyTree& _config;
    const base::KeyVault& _vault;
    const lk::Address _this_node_address;
    //==================
    base::Observable<const lk::Block&> _event_block_added;
    base::Observable<const lk::Transaction&> _event_new_pending_transaction;
    //==================
    bool _is_account_manager_updated{ false };
    AccountManager _account_manager;
    CodeManager _code_manager;
    lk::Blockchain _blockchain;
    lk::Host _host;
    //==================
    lk::EthAdapter _eth_adapter;
    //==================
    std::unordered_map<base::Sha256, base::Bytes> _tx_outputs;
    mutable std::shared_mutex _tx_outputs_mutex;
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
    std::tuple<lk::Address, base::Bytes, std::uint64_t> doContractCreation(const lk::Transaction& tx,
                                                                         const lk::Block& block_where_tx);
    vm::ExecutionResult doMessageCall(const lk::Transaction& tx, const lk::Block& block_where_tx);
    //==================
  public:
    //==================
    // notifies if new blocks are added: genesis and blocks, that are stored in DB, are not handled by this
    void subscribeToBlockAddition(decltype(_event_block_added)::CallbackType callback);

    // notifies if some transaction was added to set of pending
    void subscribeToNewPendingTransaction(decltype(_event_new_pending_transaction)::CallbackType callback);
    //==================
};

} // namespace core
