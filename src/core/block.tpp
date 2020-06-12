#pragma once

namespace lk
{

template<typename B>
BlockFieldsView<B>::BlockFieldsView(const B& block)
  : _block{ block }
{}


template<typename B>
BlockDepth BlockFieldsView<B>::getDepth() const noexcept
{
    return _block.getDepth();
}


template<typename B>
const base::Sha256& BlockFieldsView<B>::getPrevBlockHash() const noexcept
{
    return _block.getPrevBlockHash();
}


template<typename B>
const TransactionsSet& BlockFieldsView<B>::getTransactions() const noexcept
{
    return _block.getTransactions();
}


template<typename B>
NonceInt BlockFieldsView<B>::getNonce() const noexcept
{
    return _block.getNonce();
}


template<typename B>
const base::Time& BlockFieldsView<B>::getTimestamp() const noexcept
{
    return _block.Timestamp();
}


template<typename B>
const lk::Address& BlockFieldsView<B>::getCoinbase() const noexcept
{
    return _block.getCoinbase();
}

}