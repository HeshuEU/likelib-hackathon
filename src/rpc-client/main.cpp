#include "rpc/rpc.hpp"
#include "rpc/error.hpp"

#include "base/log.hpp"
#include "base/subprogram_router.hpp"
#include "base/error.hpp"
#include "base/hash.hpp"
#include "base/time.hpp"

namespace rpc_client
{

constexpr const char* HOST_ADDRESS_OPTION = "host";
constexpr const char* MONEY_OPTION = "money";
constexpr const char* ACCOUNT_ADDRESS_OPTION = "address";
constexpr const char* FROM_ACCOUNT_ADDRESS_OPTION = "from_address";
constexpr const char* TO_ACCOUNT_ADDRESS_OPTION = "to_address";

int getBalance(base::SubprogramRouter& router)
{
    router.optionsParser().addOption<std::string>(HOST_ADDRESS_OPTION, "address of remote host in format: \"<ip>:<port>\"");
    router.optionsParser().addOption<std::string>(ACCOUNT_ADDRESS_OPTION, "account address");
    router.update();

    if(router.optionsParser().hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }
    if(!router.optionsParser().hasOption(HOST_ADDRESS_OPTION)) {
        std::cout << "No option required option[" << HOST_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }
    if(!router.optionsParser().hasOption(ACCOUNT_ADDRESS_OPTION)) {
        std::cout << "No option required option[" << ACCOUNT_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }

    try {
        auto host = router.optionsParser().getValue<std::string>(HOST_ADDRESS_OPTION);
        auto address = router.optionsParser().getValue<std::string>(ACCOUNT_ADDRESS_OPTION);

        LOG_INFO << "Try to connect to rpc server by: " << host;
        rpc::RpcClient client(host);
        auto result = client.balance(address.c_str());
        std::cout << "Remote call of get_balance -> [" << result << "]" << std::endl;
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments.\n";
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cerr << "RPC error. " << er.what() << "\n";
        ;
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        LOG_ERROR << "[unknown exception caught in getBalance]";
        return base::config::EXIT_FAIL;
    }
}

int transfer(base::SubprogramRouter& router)
{
    router.optionsParser().addOption<std::string>(
        HOST_ADDRESS_OPTION, "address of remote host in format: \"<ip>:<port>\"");
    router.optionsParser().addOption<std::string>(FROM_ACCOUNT_ADDRESS_OPTION, "from account address");
    router.optionsParser().addOption<std::string>(TO_ACCOUNT_ADDRESS_OPTION, "to account address");
    router.optionsParser().addOption<std::uint32_t>(MONEY_OPTION, "transfer money");
        router.update();

    if(router.optionsParser().hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }
    if(!router.optionsParser().hasOption(HOST_ADDRESS_OPTION)) {
        std::cout << "No option required option[" << HOST_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }
    if(!router.optionsParser().hasOption(FROM_ACCOUNT_ADDRESS_OPTION)) {
        std::cout << "No option required option[" << FROM_ACCOUNT_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }
    if(!router.optionsParser().hasOption(TO_ACCOUNT_ADDRESS_OPTION)) {
        std::cout << "No option required option[" << TO_ACCOUNT_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }
    if(!router.optionsParser().hasOption(MONEY_OPTION)) {
        std::cout << "No option required option[" << MONEY_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }

    try {
        auto host = router.optionsParser().getValue<std::string>(HOST_ADDRESS_OPTION);
        auto from_address = router.optionsParser().getValue<std::string>(FROM_ACCOUNT_ADDRESS_OPTION);
        auto to_address = router.optionsParser().getValue<std::string>(TO_ACCOUNT_ADDRESS_OPTION);
        auto amount = router.optionsParser().getValue<std::uint32_t>(MONEY_OPTION);

        rpc::RpcClient client(host);
        LOG_INFO << "Try to connect to rpc server by: " << host;
        auto result = client.transaction(amount, from_address.c_str(), to_address.c_str(), base::Time::now());
        std::cout << "Remote call of transaction -> [" << result << "]" << std::endl;
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments.\n";
        LOG_ERROR << "[exception in transaction]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cerr << "RPC error. " << er.what() << "\n";
        LOG_ERROR << "[exception in transaction]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        LOG_ERROR << "[exception in transaction]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        LOG_ERROR << "[unknown exception caught in transaction]";
        return base::config::EXIT_FAIL;
    }
}

int test(base::SubprogramRouter& router)
{
    constexpr const char* HOST_ADDRESS_OPTION = "host";
    router.optionsParser().addOption<std::string>(HOST_ADDRESS_OPTION, "address of remote host in format: \"<ip>:<port>\"");
    router.update();

    if(router.optionsParser().hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(!router.optionsParser().hasOption(HOST_ADDRESS_OPTION)) {
        std::cout << "No option required option[" << HOST_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }

    try {
        auto host = router.optionsParser().getValue<std::string>("host");

        LOG_INFO << "Try to connect to rpc server by: " << host;
        rpc::RpcClient client(host);

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
        std::cerr << "Bad input arguments\n";
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cerr << "RPC error. " << er.what() << "\n";
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        LOG_ERROR << "[unknown exception caught in test]";
        return base::config::EXIT_FAIL;
    }
}

int mainProcess(base::SubprogramRouter& router)
{
    router.optionsParser().addFlag("version,v", "Print version of program");
    router.update();

    if(router.optionsParser().hasOption("help") || router.optionsParser().empty()) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(router.optionsParser().hasOption("version")) {
        std::cout << "Likelib2 rpc client 0.1" << std::endl;
        return base::config::EXIT_OK;
    }

    return base::config::EXIT_OK;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        base::initLog(base::LogLevel::ALL, base::Sink::FILE);
        base::SubprogramRouter router("rpc-client", rpc_client::mainProcess);
        router.addSubprogram("get_balance", "use for get balance from remote by account address", rpc_client::getBalance);
        router.addSubprogram("transfer", "use transfer balance from one address to another address", rpc_client::transfer);
        router.addSubprogram("test", "use test functions", rpc_client::test);
        return router.process(argc, argv);
    }
    catch(const std::exception& error) {
        LOG_ERROR << "[exception caught] " << error.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        LOG_ERROR << "[unknown exception caught]";
        return base::config::EXIT_FAIL;
    }
    return base::config::EXIT_OK;
}
