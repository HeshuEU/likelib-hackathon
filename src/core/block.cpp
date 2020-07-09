#include "block.hpp"

#include "base/hash.hpp"

#include <utility>

namespace lk
{

ImmutableBlock::ImmutableBlock(lk::BlockDepth depth,
                               NonceInt nonce,
                               base::Sha256 prev_block_hash,
                               base::Time timestamp,
                               lk::Address coinbase,
                               TransactionsSet txs)
  : _depth{ depth }
  , _nonce{ nonce }
  , _prev_block_hash{ std::move(prev_block_hash) }
  , _timestamp{ std::move(timestamp) }
  , _coinbase{ std::move(coinbase) }
  , _txs(std::move(txs))
  , _this_block_hash{ computeThisBlockHash() }
{}


base::Sha256 ImmutableBlock::computeThisBlockHash()
{
    base::SerializationOArchive oa;
    oa.serialize(*this);
    return base::Sha256::compute(std::move(oa).getBytes());
}


void ImmutableBlock::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_depth);
    oa.serialize(_nonce);
    oa.serialize(_prev_block_hash);
    oa.serialize(_timestamp);
    oa.serialize(_coinbase);
    oa.serialize(_txs);
}


ImmutableBlock ImmutableBlock::deserialize(base::SerializationIArchive& ia)
{
    auto depth = ia.deserialize<BlockDepth>();
    auto nonce = ia.deserialize<NonceInt>();
    auto prev_block_hash = ia.deserialize<base::Sha256>();
    auto timestamp = ia.deserialize<base::Time>();
    auto coinbase = ia.deserialize<lk::Address>();
    auto txs = ia.deserialize<TransactionsSet>();

    ImmutableBlock ret{ depth,         nonce, std::move(prev_block_hash), std::move(timestamp), std::move(coinbase),
                        std::move(txs) };
    return ret;
}


lk::BlockDepth ImmutableBlock::getDepth() const noexcept
{
    return _depth;
}


const base::Sha256& ImmutableBlock::getPrevBlockHash() const noexcept
{
    return _prev_block_hash;
}


const TransactionsSet& ImmutableBlock::getTransactions() const noexcept
{
    return _txs;
}


NonceInt ImmutableBlock::getNonce() const noexcept
{
    return _nonce;
}


const base::Time& ImmutableBlock::getTimestamp() const noexcept
{
    return _timestamp;
}


const lk::Address& ImmutableBlock::getCoinbase() const noexcept
{
    return _coinbase;
}


const base::Sha256& ImmutableBlock::getHash() const noexcept
{
    return _this_block_hash;
}

//=================================================

MutableBlock::MutableBlock(lk::BlockDepth depth,
                           NonceInt nonce,
                           base::Sha256 prev_block_hash,
                           base::Time timestamp,
                           lk::Address coinbase,
                           TransactionsSet txs)
  : _depth{ depth }
  , _nonce{ nonce }
  , _prev_block_hash{ std::move(prev_block_hash) }
  , _timestamp{ std::move(timestamp) }
  , _coinbase{ std::move(coinbase) }
  , _txs(std::move(txs))
{}


void MutableBlock::serialize(base::SerializationOArchive& oa) const
{
    oa.serialize(_depth);
    oa.serialize(_nonce);
    oa.serialize(_prev_block_hash);
    oa.serialize(_timestamp);
    oa.serialize(_coinbase);
    oa.serialize(_txs);
}


MutableBlock MutableBlock::deserialize(base::SerializationIArchive& ia)
{
    auto depth = ia.deserialize<BlockDepth>();
    auto nonce = ia.deserialize<NonceInt>();
    auto prev_block_hash = ia.deserialize<base::Sha256>();
    auto timestamp = ia.deserialize<base::Time>();
    auto coinbase = ia.deserialize<lk::Address>();
    auto txs = ia.deserialize<TransactionsSet>();

    MutableBlock ret{ depth,         nonce, std::move(prev_block_hash), std::move(timestamp), std::move(coinbase),
                      std::move(txs) };
    return ret;
}


lk::BlockDepth MutableBlock::getDepth() const noexcept
{
    return _depth;
}


const base::Sha256& MutableBlock::getPrevBlockHash() const noexcept
{
    return _prev_block_hash;
}


const TransactionsSet& MutableBlock::getTransactions() const noexcept
{
    return _txs;
}


NonceInt MutableBlock::getNonce() const noexcept
{
    return _nonce;
}


const base::Time& MutableBlock::getTimestamp() const noexcept
{
    return _timestamp;
}


const lk::Address& MutableBlock::getCoinbase() const noexcept
{
    return _coinbase;
}


void MutableBlock::setDepth(BlockDepth depth) noexcept
{
    _depth = depth;
}


void MutableBlock::setNonce(NonceInt nonce) noexcept
{
    _nonce = nonce;
}


void MutableBlock::setPrevBlockHash(const base::Sha256& prev_block_hash)
{
    _prev_block_hash = prev_block_hash;
}


void MutableBlock::setTimestamp(base::Time timestamp)
{
    _timestamp = std::move(timestamp);
}


void MutableBlock::setTransactions(TransactionsSet txs)
{
    _txs = std::move(txs);
}


void MutableBlock::addTransaction(const Transaction& tx)
{
    _txs.add(tx);
}

//=================================================

bool operator==(const ImmutableBlock& a, const ImmutableBlock& b)
{
    return a.getDepth() == b.getDepth() && a.getNonce() == b.getNonce() &&
           a.getPrevBlockHash() == b.getPrevBlockHash() && a.getTimestamp() == b.getTimestamp() &&
           a.getCoinbase() == b.getCoinbase() && a.getTransactions() == b.getTransactions();
    // or simply check hashes on equality?
}


bool operator!=(const ImmutableBlock& a, const ImmutableBlock& b)
{
    return !(a == b);
}


bool operator==(const MutableBlock& a, const MutableBlock& b)
{
    return a.getDepth() == b.getDepth() && a.getNonce() == b.getNonce() &&
           a.getPrevBlockHash() == b.getPrevBlockHash() && a.getTimestamp() == b.getTimestamp() &&
           a.getCoinbase() == b.getCoinbase() && a.getTransactions() == b.getTransactions();
}


bool operator!=(const MutableBlock& a, const MutableBlock& b)
{
    return !(a == b);
}

//=================================================

BlockFieldsView::BlockFieldsView(const ImmutableBlock& block)
  : _b1{ &block }
  , _b2{ nullptr }
{}


BlockFieldsView::BlockFieldsView(const MutableBlock& block)
  : _b1{ nullptr }
  , _b2{ &block }
{}


BlockDepth BlockFieldsView::getDepth() const noexcept
{
    return (_b1 ? _b1->getDepth() : _b2->getDepth());
}


const base::Sha256& BlockFieldsView::getPrevBlockHash() const noexcept
{
    return (_b1 ? _b1->getPrevBlockHash() : _b2->getPrevBlockHash());
}


const TransactionsSet& BlockFieldsView::getTransactions() const noexcept
{
    return (_b1 ? _b1->getTransactions() : _b2->getTransactions());
}


NonceInt BlockFieldsView::getNonce() const noexcept
{
    return (_b1 ? _b1->getNonce() : _b2->getNonce());
}


const base::Time& BlockFieldsView::getTimestamp() const noexcept
{
    return (_b1 ? _b1->getTimestamp() : _b2->getTimestamp());
}


const lk::Address& BlockFieldsView::getCoinbase() const noexcept
{
    return (_b1 ? _b1->getCoinbase() : _b2->getCoinbase());
}

//=================================================

BlockBuilder::BlockBuilder(const ImmutableBlock& block)
{
    initFromBlock(block);
}


BlockBuilder::BlockBuilder(const MutableBlock& block)
{
    initFromBlock(block);
}


template<typename T>
void BlockBuilder::initFromBlock(const T& b)
{
    setDepth(b.getDepth());
    setNonce(b.getNonce());
    setPrevBlockHash(b.getPrevBlockHash());
    setTimestamp(b.getTimestamp());
    setCoinbase(b.getCoinbase());
    setTransactionsSet(b.getTransactions());
}


void BlockBuilder::setDepth(BlockDepth depth)
{
    _depth = depth;
}


void BlockBuilder::setNonce(NonceInt nonce)
{
    _nonce = nonce;
}


void BlockBuilder::setPrevBlockHash(base::Sha256 hash)
{
    _prev_block_hash = std::move(hash);
}


void BlockBuilder::setTimestamp(base::Time timestamp)
{
    _timestamp = std::move(timestamp);
}


void BlockBuilder::setCoinbase(Address address)
{
    _coinbase = std::move(address);
}


void BlockBuilder::setTransactionsSet(TransactionsSet txs)
{
    _txs = std::move(txs);
}


ImmutableBlock BlockBuilder::buildImmutable() const&
{
    raiseIfNotEverythingIsSet();
    return ImmutableBlock{ *_depth, *_nonce, *_prev_block_hash, *_timestamp, *_coinbase, *_txs };
}


MutableBlock BlockBuilder::buildMutable() const&
{
    raiseIfNotEverythingIsSet();
    return MutableBlock{ *_depth, *_nonce, *_prev_block_hash, *_timestamp, *_coinbase, *_txs };
}


ImmutableBlock BlockBuilder::buildImmutable() &&
{
    raiseIfNotEverythingIsSet();
    auto ret = ImmutableBlock{
        *_depth, *_nonce, *std::move(_prev_block_hash), *std::move(_timestamp), *std::move(_coinbase), *std::move(_txs)
    };
    null();
    return ret;
}


MutableBlock BlockBuilder::buildMutable() &&
{
    raiseIfNotEverythingIsSet();
    auto ret = MutableBlock{
        *_depth, *_nonce, *std::move(_prev_block_hash), *std::move(_timestamp), *std::move(_coinbase), *std::move(_txs)
    };
    null();
    return ret;
}


void BlockBuilder::raiseIfNotEverythingIsSet() const
{
    if (!(_depth && _nonce && _prev_block_hash && _timestamp && _coinbase && _txs)) {
        RAISE_ERROR(base::UseOfUninitializedValue, "cannot build block if not all fields are set up");
        // FIX DB LOADING AND SAVING (IT MIGHT BE GENESIS BLOCK PROBLEM)
    }
}


void BlockBuilder::null()
{
    _depth = std::nullopt;
    _nonce = std::nullopt;
    _prev_block_hash = std::nullopt;
    _timestamp = std::nullopt;
    _coinbase = std::nullopt;
    _txs = std::nullopt;
}

} // namespace lk
