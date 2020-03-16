#pragma once

#include "base/property_tree.hpp"
#include "base/crypto.hpp"
#include "base/utility.hpp"
#include "bc/block.hpp"
#include "bc/blockchain.hpp"
#include "lk/eth_adapter.hpp"
#include "lk/managers.hpp"
#include "lk/protocol.hpp"
#include "net/host.hpp"

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
    bc::Balance getBalance(const bc::Address& address) const;
    //==================
    bool addPendingTransaction(const bc::Transaction& tx);
    void addPendingTransactionAndWait(const bc::Transaction& tx);
    base::Bytes getTransactionOutput(const base::Sha256& tx_hash);
    //==================
    bool tryAddBlock(const bc::Block& b);
    std::optional<bc::Block> findBlock(const base::Sha256& hash) const;
    std::optional<base::Sha256> findBlockHash(const bc::BlockDepth& depth) const;
    const bc::Block& getTopBlock() const;
    //==================
    bc::Block getBlockTemplate() const;
    //==================
    const bc::Address& getThisNodeAddress() const noexcept;
    //==================
  private:
    //==================
    friend class lk::EthAdapter;
    //==================
    const base::PropertyTree& _config;
    const base::KeyVault& _vault;
    const bc::Address _this_node_address;
    //==================
    base::Observable<const bc::Block&> _event_block_added;
    base::Observable<const bc::Transaction&> _event_new_pending_transaction;
    //==================
    bool _is_account_manager_updated{false};
    AccountManager _account_manager;
    CodeManager _code_manager;
    bc::Blockchain _blockchain;
    lk::Network _network;
    //==================
    lk::EthAdapter _eth_adapter;
    //==================
    std::unordered_map<base::Sha256, base::Bytes> _tx_outputs;
    mutable std::shared_mutex _tx_outputs_mutex;
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
    bool tryPerformTransaction(const bc::Transaction& tx, const bc::Block& block_where_tx);
    std::tuple<bc::Address, base::Bytes, bc::Balance> doContractCreation(const bc::Transaction& tx, const bc::Block& block_where_tx);
    vm::ExecutionResult doMessageCall(const bc::Transaction& tx, const bc::Block& block_where_tx);
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
