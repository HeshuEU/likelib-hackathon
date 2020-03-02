#include "actions.hpp"
#include "config.hpp"
#include "utility.hpp"

#include "base/error.hpp"
#include "base/log.hpp"
#include "base/subprogram_router.hpp"

#include <iostream>


int mainProcess(base::SubprogramRouter& router)
{
    router.getOptionsParser()->addFlag("version,v", "Print version of program");
    router.update();

    if(router.getOptionsParser()->hasOption("help") || router.getOptionsParser()->empty()) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(router.getOptionsParser()->hasOption("version")) {
        std::cout << "Likelib2 client " << config::VERSION << std::endl;
        return base::config::EXIT_OK;
    }

    return base::config::EXIT_OK;
}


int main(int argc, char** argv)
{
    try {
        base::initLog(base::Sink::FILE);
        base::SubprogramRouter router("client", mainProcess);
        router.addSubprogram("generate", "generate a pair of keys", generateKeys);
        router.addSubprogram("get_balance", "use for get balance from remote by account address", getBalance);
        router.addSubprogram("transfer", "use transfer balance from one address to another address", transfer);
        router.addSubprogram("test", "test RPC connection", testConnection);
        router.addSubprogram("create_contract", "deploy a smart contract", createContract);
        router.addSubprogram("message_call", "create message to call smart contract", messageCall);
        router.addSubprogram("compile", "compile smart contract", compileCode);
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
