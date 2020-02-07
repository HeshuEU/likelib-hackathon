#include "parameters_helper.hpp"

#include "rpc/rpc.hpp"
#include "rpc/error.hpp"

#include "base/log.hpp"
#include "base/subprogram_router.hpp"
#include "base/error.hpp"
#include "base/time.hpp"
#include "base/property_tree.hpp"
#include "base/config.hpp"

namespace rpc_client
{

static constexpr const char* DEFAULT_CONFIG_PATH = "config.json";
static constexpr uint32_t API_VERSION = base::config::RPC_PUBLIC_API_VERSION;

int getBalance(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, DEFAULT_CONFIG_PATH, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* ADDRESS_OPTION = "address";
    router.optionsParser()->addOption<std::string>(ADDRESS_OPTION, "address of target account");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION)};
        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)) {
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        }
        else {
            host_address = helper.getValue<std::string>("nodes", "host address");
        }
        //====================================
        std::string account_address;
        if(router.optionsParser()->hasOption(ADDRESS_OPTION)) {
            account_address = router.optionsParser()->getValue<std::string>(ADDRESS_OPTION);
        }
        else {
            account_address = helper.getValue<std::string>("addresses", "account address");
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
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, DEFAULT_CONFIG_PATH, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* FROM_ADDRESS_OPTION = "from";
    router.optionsParser()->addOption<std::string>(FROM_ADDRESS_OPTION, "address of \"from\" account");
    constexpr const char* TO_ADDRESS_OPTION = "to";
    router.optionsParser()->addOption<std::string>(TO_ADDRESS_OPTION, "address of \"to\" account");
    constexpr const char* AMOUNT_OPTION = "amount";
    router.optionsParser()->addOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    constexpr const char* FEE_OPTION = "fee";
    router.optionsParser()->addOption<bc::Balance>(FEE_OPTION, "fee count");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION)};

        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)) {
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        }
        else {
            host_address = helper.getValue<std::string>("nodes", "host address");
        }
        //====================================
        std::string from_address;
        if(router.optionsParser()->hasOption(FROM_ADDRESS_OPTION)) {
            from_address = router.optionsParser()->getValue<std::string>(FROM_ADDRESS_OPTION);
        }
        else {
            from_address = helper.getValue<std::string>("addresses", "from account address");
        }
        //====================================
        std::string to_address;
        if(router.optionsParser()->hasOption(TO_ADDRESS_OPTION)) {
            to_address = router.optionsParser()->getValue<std::string>(TO_ADDRESS_OPTION);
        }
        else {
            to_address = helper.getValue<std::string>("addresses", "to account address");
        }
        //====================================
        bc::Balance amount;
        if(router.optionsParser()->hasOption(AMOUNT_OPTION)) {
            amount = router.optionsParser()->getValue<bc::Balance>(AMOUNT_OPTION);
        }
        else {
            amount = helper.getValue<bc::Balance>("amount", "transfer amount");
        } //====================================
        bc::Balance fee;
        if(router.optionsParser()->hasOption(FEE_OPTION)) {
            fee = router.optionsParser()->getValue<bc::Balance>(FEE_OPTION);
        }
        else {
            fee = helper.getValue<bc::Balance>("fee", "transfer fee");
        }
        //====================================
        rpc::RpcClient client(host_address);
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        auto result =
            client.transaction_to_wallet(amount, from_address.c_str(), to_address.c_str(), fee, base::Time::now());

        if(result) {
            std::cout << "Remote call of transaction from account to account success -> [" << result.getMessage() << "]" << std::endl;
            return base::config::EXIT_OK;
        }
        else {
            std::cout << "Remote call of transaction from account to account is failed -> [" << result.getMessage() << "]" << std::endl;
            return base::config::EXIT_FAIL;
        }
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

int testConnection(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, DEFAULT_CONFIG_PATH, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* CODE_PATH_OPTION = "code_path";
    router.optionsParser()->addOption<std::string>(CODE_PATH_OPTION, "path to compiled code");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION)};

        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)) {
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        }
        else {
            host_address = helper.getValue<std::string>("nodes", "host address");
        }
        //====================================
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);

        auto answer = client.test(API_VERSION);

        if(answer) {
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


int pushContract(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, DEFAULT_CONFIG_PATH, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* CODE_PATH_OPTION = "code";
    router.optionsParser()->addOption<std::string>(CODE_PATH_OPTION, "path to compiled code");
    constexpr const char* CODE_REVISION_OPTION = "code";
    router.optionsParser()->addOption<std::uint32_t>(CODE_REVISION_OPTION, "revision of compiled code");
    constexpr const char* FROM_ADDRESS_OPTION = "from";
    router.optionsParser()->addOption<std::string>(FROM_ADDRESS_OPTION, "address of \"from\" account");
    constexpr const char* AMOUNT_OPTION = "amount";
    router.optionsParser()->addOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    constexpr const char* GAS_OPTION = "gas";
    router.optionsParser()->addOption<bc::Balance>(GAS_OPTION, "gas count");
    constexpr const char* INITIAL_MESSAGE_OPTION = "initial_message";
    router.optionsParser()->addOption<std::string>(INITIAL_MESSAGE_OPTION, "message for initialize smart contract");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION)};
        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)) {
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        }
        else {
            host_address = helper.getValue<std::string>("nodes", "host address");
        }
        //====================================
        std::string file_path;
        if(router.optionsParser()->hasOption(CODE_PATH_OPTION)) {
            file_path = router.optionsParser()->getValue<std::string>(CODE_PATH_OPTION);
        }
        else {
            file_path = helper.getValue<std::string>("code_paths", "path to compiled smart contract");
            if(file_path.size() == 0) {
                std::cout << "Path to code not set up" << std::endl;
                return base::config::EXIT_FAIL;
            }
        }
        base::Bytes contract;
        try {
            auto compiled_code = base::readConfig(file_path);
            auto contract_data = compiled_code.get<std::string>("object");
            contract = base::Bytes::fromHex(contract_data);
        }
        catch(const base::InaccessibleFile& er) {
            std::cout << "File no found by path: " << file_path << std::endl;
            return base::config::EXIT_FAIL;
        }
        catch(const base::ParsingError& er) {
            std::cout << "File no found by path: " << file_path << std::endl;
            LOG_ERROR << "[not json valid format]" << er.what();
            return base::config::EXIT_FAIL;
        }
        catch(const base::InvalidArgument& er) {
            std::cout << "Code not found at file: " << file_path << std::endl;
            LOG_ERROR << "[no valid data file" << file_path << "]" << er.what();
            return base::config::EXIT_FAIL;
        }
        //====================================
        std::uint32_t revision;
        if(router.optionsParser()->hasOption(CODE_REVISION_OPTION)) {
            revision = router.optionsParser()->getValue<std::uint32_t>(CODE_REVISION_OPTION);
        }
        else {
            revision = helper.getValue<std::uint32_t>("revision", "code build revision");
        }
        //====================================
        std::string from_address;
        if(router.optionsParser()->hasOption(FROM_ADDRESS_OPTION)) {
            from_address = router.optionsParser()->getValue<std::string>(FROM_ADDRESS_OPTION);
        }
        else {
            from_address = helper.getValue<std::string>("addresses", "from account address");
        }
        //====================================
        bc::Balance amount;
        if(router.optionsParser()->hasOption(AMOUNT_OPTION)) {
            amount = router.optionsParser()->getValue<bc::Balance>(AMOUNT_OPTION);
        }
        else {
            amount = helper.getValue<bc::Balance>("amount", "transfer amount");
        }
        //====================================
        bc::Balance gas;
        if(router.optionsParser()->hasOption(GAS_OPTION)) {
            gas = router.optionsParser()->getValue<bc::Balance>(GAS_OPTION);
        }
        else {
            gas = helper.getValue<bc::Balance>("gas", "gas count to evaluate contract initialization");
        }
        //====================================
        std::string message_data;
        if(router.optionsParser()->hasOption(INITIAL_MESSAGE_OPTION)) {
            message_data = router.optionsParser()->getValue<std::string>(INITIAL_MESSAGE_OPTION);
        }
        else {
            message_data = helper.getValue<std::string>("messages", "message for initialization smart contract");
        }
        base::Bytes message = base::Bytes::fromHex(message_data);
        //====================================
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);

        rpc::OperationStatus status = rpc::OperationStatus::createFailed();
        bc::Address contract_address;
        bc::Balance gas_left;
        std::tie(status, contract_address, gas_left) = client.transaction_creation_contract(
            amount, from_address, base::Time::now(), gas, revision, contract, message);

        if(status) {
            std::cout << "Remote call of creation smart contract success -> [" << status.getMessage() << "], contract created at ["
                      << contract_address.toString() << "], gas left[" << gas_left << "]" << std::endl;
            return base::config::EXIT_OK;
        }
        else {
            std::cout << "Remote call of creation smart contract is failed -> [" << status.getMessage() << "]" << std::endl;
            return base::config::EXIT_FAIL;
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


int messageToContract(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, DEFAULT_CONFIG_PATH, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* FROM_ADDRESS_OPTION = "from";
    router.optionsParser()->addOption<std::string>(FROM_ADDRESS_OPTION, "address of \"from\" account");
    constexpr const char* TO_ADDRESS_OPTION = "to";
    router.optionsParser()->addOption<std::string>(FROM_ADDRESS_OPTION, "address of \"to\" contract");
    constexpr const char* AMOUNT_OPTION = "amount";
    router.optionsParser()->addOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    constexpr const char* GAS_OPTION = "gas";
    router.optionsParser()->addOption<bc::Balance>(GAS_OPTION, "gas count");
    constexpr const char* INITIAL_MESSAGE_OPTION = "initial_message";
    router.optionsParser()->addOption<std::string>(INITIAL_MESSAGE_OPTION, "message for initialize smart contract");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION)};
        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)) {
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        }
        else {
            host_address = helper.getValue<std::string>("nodes", "host address");
        }
        //====================================
        std::string from_address;
        if(router.optionsParser()->hasOption(FROM_ADDRESS_OPTION)) {
            from_address = router.optionsParser()->getValue<std::string>(FROM_ADDRESS_OPTION);
        }
        else {
            from_address = helper.getValue<std::string>("addresses", "from account address");
        }
        //====================================
        std::string to_address;
        if(router.optionsParser()->hasOption(TO_ADDRESS_OPTION)) {
            to_address = router.optionsParser()->getValue<std::string>(TO_ADDRESS_OPTION);
        }
        else {
            to_address = helper.getValue<std::string>("addresses", "to account address");
        }
        //====================================
        bc::Balance amount;
        if(router.optionsParser()->hasOption(AMOUNT_OPTION)) {
            amount = router.optionsParser()->getValue<bc::Balance>(AMOUNT_OPTION);
        }
        else {
            amount = helper.getValue<bc::Balance>("amount", "transfer amount");
        }
        //====================================
        bc::Balance gas;
        if(router.optionsParser()->hasOption(GAS_OPTION)) {
            gas = router.optionsParser()->getValue<bc::Balance>(GAS_OPTION);
        }
        else {
            gas = helper.getValue<bc::Balance>("gas", "gas count to evaluate contract message");
        }
        //====================================
        std::string message_data;
        if(router.optionsParser()->hasOption(INITIAL_MESSAGE_OPTION)) {
            message_data = router.optionsParser()->getValue<std::string>(INITIAL_MESSAGE_OPTION);
        }
        else {
            message_data = helper.getValue<std::string>("messages", "compiled message to smart contract");
        }
        base::Bytes message = base::Bytes::fromHex(message_data);
        //====================================
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);

        rpc::OperationStatus status = rpc::OperationStatus::createFailed();
        base::Bytes contract_response;
        bc::Balance gas_left;
        std::tie(status, contract_response, gas_left) = client.transaction_to_contract(
            amount, from_address, to_address, base::Time::now(), gas, message);

        if(status) {
            std::cout << "Remote call of smart contract call success -> [" << status.getMessage() << "], contract response["
                      << contract_response.toHex() << "], gas left[" << gas_left << "]" << std::endl;
            return base::config::EXIT_OK;
        }
        else {
            std::cout << "Remote call of smart contract call is failed -> [" << status.getMessage() << "]" << std::endl;
            return base::config::EXIT_FAIL;
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
        router.addSubprogram("test", "use test functions", rpc_client::testConnection);
        router.addSubprogram("push_contract", "load smart contract code to push to blockchain network", rpc_client::pushContract);
        router.addSubprogram("message_to_contract", "create message to call smart contract ", rpc_client::messageToContract);

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
