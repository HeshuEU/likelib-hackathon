#include "parameters_helper.hpp"

#include "rpc/rpc.hpp"
#include "rpc/error.hpp"

#include "base/log.hpp"
#include "base/subprogram_router.hpp"
#include "base/error.hpp"
#include "base/hash.hpp"
#include "base/time.hpp"

namespace rpc_client
{

int getBalance(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* ADDRESS_OPTION = "address";
    router.optionsParser()->addOption<std::string>(ADDRESS_OPTION, "address of target account");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(!router.optionsParser()->hasOption(CONFIG_OPTION)) {
        std::cout << "Warning. Config was not found." << std::endl;
    }

    try {
        std::unique_ptr<ParametersHelper> helper;
        if(router.optionsParser()->hasOption(CONFIG_OPTION)) {
            helper = std::make_unique<ParametersHelper>(router.optionsParser()->getValue<std::string>(CONFIG_OPTION));
        }
        else {
            helper = std::make_unique<ParametersHelper>();
        }
        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)){
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        } else {
            host_address = helper->getValue<std::string>("nodes", "host address");
        }
        //====================================
        std::string account_address;
        if(router.optionsParser()->hasOption(ADDRESS_OPTION)){
            account_address = router.optionsParser()->getValue<std::string>(ADDRESS_OPTION);
        } else {
            account_address = helper->getValue<std::string>("addresses", "account address");
        }
        //====================================

        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);
        auto result = client.balance(account_address.c_str());
        std::cout << "Remote call of get_balance -> [" << result << "]" << std::endl;
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments.\n" << router.helpMessage();
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cerr << "RPC error. " << er.what() << "\n";
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        std::cerr << "Unexpected error." << er.what() << "\n";
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        std::cerr << "Unexpected error.\n";
        LOG_ERROR << "[unknown exception caught in test]";
        return base::config::EXIT_FAIL;
    }
}

int transfer(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* FROM_ADDRESS_OPTION = "from";
    router.optionsParser()->addOption<std::string>(FROM_ADDRESS_OPTION, "address of \"from\" account");
    constexpr const char* TO_ADDRESS_OPTION = "to";
    router.optionsParser()->addOption<std::string>(TO_ADDRESS_OPTION, "address of \"to\" account");
    constexpr const char* AMOUNT_OPTION = "amount";
    router.optionsParser()->addOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(!router.optionsParser()->hasOption(CONFIG_OPTION)) {
        std::cout << "Warning. Config was not found." << std::endl;
    }

    try {
        std::unique_ptr<ParametersHelper> helper;
        if(router.optionsParser()->hasOption(CONFIG_OPTION)) {
            helper = std::make_unique<ParametersHelper>(router.optionsParser()->getValue<std::string>(CONFIG_OPTION));
        }
        else {
            helper = std::make_unique<ParametersHelper>();
        }

        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)){
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        } else {
            host_address = helper->getValue<std::string>("nodes", "host address");
        }
        //====================================
        std::string from_address;
        if(router.optionsParser()->hasOption(FROM_ADDRESS_OPTION)){
            from_address = router.optionsParser()->getValue<std::string>(FROM_ADDRESS_OPTION);
        } else {
            from_address = helper->getValue<std::string>("addresses", "from account address");
        }
        //====================================
        std::string to_address;
        if(router.optionsParser()->hasOption(TO_ADDRESS_OPTION)){
            to_address = router.optionsParser()->getValue<std::string>(TO_ADDRESS_OPTION);
        } else {
            to_address = helper->getValue<std::string>("addresses", "to account address");
        }
        //====================================
        bc::Balance amount;
        if(router.optionsParser()->hasOption(AMOUNT_OPTION)){
            amount = router.optionsParser()->getValue<bc::Balance>(AMOUNT_OPTION);
        } else {
            amount = helper->getValue<bc::Balance>("amount", "transfer amount");
        }
        //====================================
        rpc::RpcClient client(host_address);
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        auto result = client.transaction(amount, from_address.c_str(), to_address.c_str(), base::Time::now());
        std::cout << "Remote call of transaction -> [" << result << "]" << std::endl;
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments.\n" << router.helpMessage();
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cerr << "RPC error. " << er.what() << "\n";
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        std::cerr << "Unexpected error." << er.what() << "\n";
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        std::cerr << "Unexpected error.\n";
        LOG_ERROR << "[unknown exception caught in test]";
        return base::config::EXIT_FAIL;
    }
}

int test(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(!router.optionsParser()->hasOption(CONFIG_OPTION)) {
        std::cout << "Warning. Config was not found." << std::endl;
    }

    try {
        std::unique_ptr<ParametersHelper> helper;
        if(router.optionsParser()->hasOption(CONFIG_OPTION)) {
            helper = std::make_unique<ParametersHelper>(router.optionsParser()->getValue<std::string>(CONFIG_OPTION));
        }
        else {
            helper = std::make_unique<ParametersHelper>();
        }
        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)){
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        } else {
            host_address = helper->getValue<std::string>("nodes", "host address");
        }
        //====================================
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);

        auto data = base::Sha256::compute(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_REQUEST)).toHex();
        auto answer = client.test(data);
        auto our_answer = base::Sha256::compute(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_RESPONSE)).toHex();

        if(answer == our_answer) {
            std::cout << "Test passed" << std::endl;
        }
        else {
            std::cout << "Test failed" << std::endl;
        }
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments\n" << router.helpMessage();
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cerr << "RPC error. " << er.what() << "\n";
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        std::cerr << "Unexpected error." << er.what() << "\n";
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        std::cerr << "Unexpected error.\n";
        LOG_ERROR << "[unknown exception caught in test]";
        return base::config::EXIT_FAIL;
    }
}

int mainProcess(base::SubprogramRouter& router)
{
    router.optionsParser()->addFlag("version,v", "Print version of program");
    router.update();

    if(router.optionsParser()->hasOption("help") || router.optionsParser()->empty()) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(router.optionsParser()->hasOption("version")) {
        std::cout << "Likelib2 rpc client 0.1" << std::endl;
        return base::config::EXIT_OK;
    }

    return base::config::EXIT_OK;
}

} // namespace rpc_client

int main(int argc, char** argv)
{
    try {
        base::initLog(base::LogLevel::ALL, base::Sink::FILE);
        base::SubprogramRouter router("rpc-client", rpc_client::mainProcess);
        router.addSubprogram(
            "get_balance", "use for get balance from remote by account address", rpc_client::getBalance);
        router.addSubprogram(
            "transfer", "use transfer balance from one address to another address", rpc_client::transfer);
        router.addSubprogram("test", "use test functions", rpc_client::test);
        return router.process(argc, argv);
    }
    catch(const std::exception& error) {
        std::cerr << "Unexpected error." << error.what() << "\n";
        LOG_ERROR << "[exception caught] " << error.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        std::cerr << "Unexpected error.\n";
        LOG_ERROR << "[unknown exception caught]";
        return base::config::EXIT_FAIL;
    }
}
