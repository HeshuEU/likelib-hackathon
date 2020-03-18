#include "actions.hpp"
#include "config.hpp"
#include "utility.hpp"

#include "base/error.hpp"
#include "base/log.hpp"
#include "base/subprogram_router.hpp"

#include <iostream>


int mainProcess(base::SubprogramRouter& router)
{
    router.getOptionsParser().addFlag("version,v", "Print version of program");
    router.update();

    if(router.getOptionsParser().hasOption("help") || router.getOptionsParser().empty()) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(router.getOptionsParser().hasOption("version")) {
        std::cout << "Likelib2 client " << config::VERSION << std::endl;
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
        router.addSubprogram("generate", "generate a pair of keys", run<ActionGenerateKeys>);
        router.addSubprogram("keys_info", "show info on keys", run<ActionKeysInfo>);
        router.addSubprogram("info", "get LK info", run<ActionInfo>);
        router.addSubprogram("get_block", "get block information", run<ActionGetBlock>);
        router.addSubprogram(
            "get_balance", "use for get balance from remote by account address", run<ActionGetBalance>);
        router.addSubprogram(
            "transfer", "use transfer balance from one address to another address", run<ActionTransfer>);
        router.addSubprogram("test", "test RPC connection", run<ActionTestConnection>);
        router.addSubprogram("create_contract", "deploy a smart contract", run<ActionCreateContract>);
        router.addSubprogram("message_call", "create message to call smart contract", run<ActionMessageCall>);
        router.addSubprogram("compile", "compile smart contract", run<ActionCompile>);
        router.addSubprogram("encode", "encode smart contract message", run<ActionEncode>);
        return router.process(argc, argv);
    }
    catch(const std::exception& error) {
        printUnexpectedError(error.what());
        LOG_ERROR << "[exception caught] " << error.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        printUnexpectedError();
        LOG_ERROR << "[unknown exception caught]";
        return base::config::EXIT_FAIL;
    }
}
