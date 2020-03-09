#pragma once

#include "base/bytes.hpp"
#include "bc/block.hpp"
#include "lk/managers.hpp"

#include <memory>

namespace lk
{

class Core;

class EthAdapter
{
  public:
    EthAdapter(base::Bytes code, Core& core, const bc::Transaction& associated_tx, const bc::Block& associated_block,
        AccountManager& account_manager, CodeManager& code_manager);

  private:
    class ContractRunnerImpl;
    std::unique_ptr<ContractRunnerImpl> _impl;
};

} // namespace lk