#pragma once

#include "base/bytes.hpp"
#include "bc/block.hpp"
#include "lk/managers.hpp"
#include "vm/vm.hpp"

#include <memory>
#include <mutex>

namespace lk
{

class Core;

class EthAdapter
{
  public:
    EthAdapter(Core& core, AccountManager& account_manager, CodeManager& code_manager);

    ~EthAdapter();

    std::tuple<bc::Address, base::Bytes, bc::Balance> createContract(const bc::Address& bc_address,
                                                                     const bc::Transaction& associated_tx,
                                                                     const bc::Block& associated_block);
    vm::ExecutionResult call(const bc::Transaction& associated_tx, const bc::Block& associated_block);

  private:
    class EthHost;
    std::unique_ptr<EthHost> _eth_host;

    vm::Vm _vm;
    Core& _core;
    AccountManager& _account_manager;
    CodeManager& _code_manager;

    mutable std::mutex _execution_mutex;
};

} // namespace lk