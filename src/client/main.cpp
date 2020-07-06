#include "actions.hpp"
#include "config.hpp"
#include "subprogram_router.hpp"
#include "utility.hpp"

#include "base/error.hpp"
#include "base/log.hpp"

#include <iostream>


int mainProcess(base::SubprogramRouter& router)
{
    router.getOptionsParser().addFlag("version,v", "Print version of program");
    router.update();

    if (router.getOptionsParser().hasOption("help") || router.getOptionsParser().empty()) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if (router.getOptionsParser().hasOption("version")) {
        std::cout << "Likelib2 client " << config::CLIENT_VERSION << std::endl;
        return base::config::EXIT_OK;
    }

    return base::config::EXIT_OK;
}


template<typename T>
int run(base::SubprogramRouter& router)
{
    T action(router);
    return action.run();
}


int main(int argc, char** argv)
{
    try {
        base::initLog(base::Sink::FILE);
        base::SubprogramRouter router("client", mainProcess);

        router.addSubprogram("connection_test", "test RPC connection", run<ActionTestConnection>);
        router.addSubprogram("node_info", "get LK info", run<ActionNodeInfo>);

        router.addSubprogram("generate_keys", "generate a pair of keys", run<ActionGenerateKeys>);
        router.addSubprogram("keys_info", "show info on keys", run<ActionKeysInfo>);

        router.addSubprogram(
          "get_balance", "use for get balance from remote by account address", run<ActionGetBalance>);
        router.addSubprogram(
          "get_account_info", "use for get account info from remote by account address", run<ActionGetAccountInfo>);

        router.addSubprogram("compile_contract", "compile smart contract", run<ActionCompile>);
        router.addSubprogram("encode_message", "encode smart contract message", run<ActionEncode>);
        router.addSubprogram("decode_message", "decode smart contract message", run<ActionDecode>);

        router.addSubprogram(
          "transfer", "use transfer balance from one address to another address", run<ActionTransfer>);
        router.addSubprogram("push_contract", "deploy a smart contract", run<ActionPushContract>);
        router.addSubprogram("call_contract", "create message to call smart contract", run<ActionContractCall>);

        router.addSubprogram("get_transaction", "get transaction information", run<ActionGetTransaction>);
        router.addSubprogram(
          "get_transaction_status", "get transaction result information", run<ActionGetTransactionStatus>);
        router.addSubprogram("get_block", "get block information", run<ActionGetBlock>);

        return router.process(argc, argv);
    }
    catch (const std::exception& error) {
        printUnexpectedError(error.what());
        LOG_ERROR << "[exception caught] " << error.what();
        return base::config::EXIT_FAIL;
    }
    catch (...) {
        printUnexpectedError();
        LOG_ERROR << "[unknown exception caught]";
        return base::config::EXIT_FAIL;
    }
}
