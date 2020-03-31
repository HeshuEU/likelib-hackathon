#pragma once

#include "base/bytes.hpp"
#include "block.hpp"
#include "core/managers.hpp"
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

    std::tuple<lk::Address, base::Bytes, lk::Balance> createContract(const lk::Address& bc_address,
                                                                     const lk::Transaction& associated_tx,
                                                                     const lk::Block& associated_block);
    vm::ExecutionResult call(const lk::Transaction& associated_tx, const lk::Block& associated_block);

  private:
    class EthHost;
    std::unique_ptr<EthHost> _eth_host;

    vm::Vm _vm;
    Core& _core;
    AccountManager& _account_manager;
    CodeManager& _code_manager;

    mutable std::recursive_mutex _call_mutex;
    mutable std::mutex _create_mutex;
};

} // namespace core