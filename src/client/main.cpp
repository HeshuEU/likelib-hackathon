#include "parameters_helper.hpp"

#include "rpc/rpc.hpp"
#include "rpc/error.hpp"

#include "base/log.hpp"
#include "base/subprogram_router.hpp"
#include "base/error.hpp"
#include "base/hash.hpp"
#include "base/time.hpp"
#include "base/property_tree.hpp"
#include "base/config.hpp"
#include "bc/transaction.hpp"
#include "vm/messages.hpp"


namespace
{
constexpr const char* const DEFAULT_CONFIG_PATH = "config.json";
constexpr uint32_t API_VERSION = base::config::RPC_PUBLIC_API_VERSION;

} // namespace


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
        std::filesystem::path config_file_path = router.optionsParser()->getValue<std::string>(CONFIG_OPTION);
        ParametersHelper helper{config_file_path, DEFAULT_CONFIG_PATH};

        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)) {
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        }
        else {
            host_address = helper.getValue<std::string>("nodes", "host address");
        }
        //====================================
        std::string account_address_str;
        if(router.optionsParser()->hasOption(ADDRESS_OPTION)) {
            account_address_str = router.optionsParser()->getValue<std::string>(ADDRESS_OPTION);
        }
        else {
            account_address_str = helper.getValue<std::string>("addresses", "account address");
        }
        bc::Address account_address{account_address_str};
        //====================================

        LOG_INFO << "GetBalance for address " << account_address;
        LOG_INFO << "Trying to connect to rpc server at " << host_address;
        rpc::RpcClient client(host_address);
        auto result = client.balance(account_address);
        std::cout << "balance of " << account_address << " is " << result << std::endl;
        LOG_INFO << "Remote call of getBalance(" << account_address << ") -> [" << result << "]";
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments.\n" << router.helpMessage();
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cerr << "RPC error: " << er.what() << '\n';
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        std::cerr << "Error: " << er.what() << '\n';
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        std::cerr << "Unexpected error.\n";
        LOG_ERROR << "[unknown exception caught in getBalance]";
        return base::config::EXIT_FAIL;
    }
}

int transfer(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, DEFAULT_CONFIG_PATH, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* TO_ADDRESS_OPTION = "to";
    router.optionsParser()->addOption<std::string>(TO_ADDRESS_OPTION, "address of recipient account");
    constexpr const char* AMOUNT_OPTION = "amount";
    router.optionsParser()->addOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    constexpr const char* KEYS_DIRECTORY_OPTION = "keys";
    router.optionsParser()->addOption<std::string>(KEYS_DIRECTORY_OPTION, "path to a directory with keys");
    constexpr const char* FEE_OPTION = "fee";
    router.optionsParser()->addOption<bc::Balance>(FEE_OPTION, "fee count");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        std::filesystem::path config_file_path = router.optionsParser()->getValue<std::string>(CONFIG_OPTION);
        ParametersHelper helper{config_file_path, DEFAULT_CONFIG_PATH};

        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)) {
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        }
        else {
            host_address = helper.getValue<std::string>("nodes", "host address");
        }
        //====================================
        std::string to_address_str;
        if(router.optionsParser()->hasOption(TO_ADDRESS_OPTION)) {
            to_address_str = router.optionsParser()->getValue<std::string>(TO_ADDRESS_OPTION);
        }
        else {
            to_address_str = helper.getValue<std::string>("addresses", "to account address");
        }
        bc::Address to_address{to_address_str};
        //====================================
        bc::Balance amount;
        if(router.optionsParser()->hasOption(AMOUNT_OPTION)) {
            amount = router.optionsParser()->getValue<bc::Balance>(AMOUNT_OPTION);
        }
        else {
            amount = helper.getValue<bc::Balance>("amount", "transfer amount");
        }
        //====================================
        bc::Balance fee;
        if(router.optionsParser()->hasOption(FEE_OPTION)) {
            fee = router.optionsParser()->getValue<bc::Balance>(FEE_OPTION);
        }
        else {
            fee = helper.getValue<bc::Balance>("fee", "transfer fee");
        }
        //====================================
        std::filesystem::path keys_path;
        if(router.optionsParser()->hasOption(KEYS_DIRECTORY_OPTION)) {
            keys_path = router.optionsParser()->getValue<std::string>(KEYS_DIRECTORY_OPTION);
        }
        else {
            keys_path = helper.getValue<std::string>("keys", "path to a directory with keys");
        }
        //====================================
        auto public_key_path = base::config::makePublicKeyPath(keys_path);
        if(!std::filesystem::exists(public_key_path)) {
            std::cerr << "Error: public key file not found at " << public_key_path;
            LOG_ERROR << "error: public key not found by path " << public_key_path;
            return base::config::EXIT_FAIL;
        }
        auto public_key = base::RsaPublicKey::load(public_key_path);
        auto from_address = bc::Address::fromPublicKey(public_key);

        auto private_key_path = base::config::makePrivateKeyPath(keys_path);
        if(!std::filesystem::exists(private_key_path)) {
            std::cerr << "Error: private key file not found at " << private_key_path;
            LOG_ERROR << "error: private key not found by path " << private_key_path;
            return base::config::EXIT_FAIL;
        }
        auto private_key = base::RsaPrivateKey::load(private_key_path);
        //====================================
        rpc::RpcClient client(host_address);
        LOG_INFO << "Transfer from " << from_address << " to " << to_address << " with amount " << amount
                 << " to rpc server " << host_address;
        LOG_INFO << "Try to connect to rpc a server at " << host_address;
        bc::TransactionBuilder txb;
        txb.setFrom(std::move(from_address));
        txb.setTo(std::move(to_address));
        txb.setAmount(amount);
        txb.setTimestamp(base::Time::now());
        txb.setFee(fee);
        auto tx = std::move(txb).build();
        tx.sign(public_key, private_key);
        auto result = client.transaction_to_wallet(tx.getAmount(), tx.getFrom(), tx.getTo(), tx.getTimestamp(), tx.getFee(), tx.getSign());
        std::cout << result << std::endl;
        LOG_INFO << "Remote call of transaction -> [" << result << "]";
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
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION), DEFAULT_CONFIG_PATH};

        //====================================
        std::string host_address;
        if(router.optionsParser()->hasOption(HOST_OPTION)) {
            host_address = router.optionsParser()->getValue<std::string>(HOST_OPTION);
        }
        else {
            host_address = helper.getValue<std::string>("nodes", "host address");
        }
        //====================================
        LOG_INFO << "Test connect to rpc server by: " << host_address;
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);

        auto answer = client.test(API_VERSION);

        if(answer) {
            std::cout << "Test passed" << std::endl;
            LOG_INFO << "Test passed";
        }
        else {
            std::cout << "Test failed" << std::endl;
            LOG_INFO << "Test failed";
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
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION), DEFAULT_CONFIG_PATH};
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
            amount, bc::Address{from_address}, base::Time::now(), gas, contract, message, bc::Sign{});

        if(status) {
            std::cout << "Remote call of creation smart contract success -> [" << status.getMessage()
                      << "], contract created at [" << contract_address.toString() << "], gas left[" << gas_left << "]"
                      << std::endl;
            return base::config::EXIT_OK;
        }
        else {
            std::cout << "Remote call of creation smart contract is failed -> [" << status.getMessage() << "]"
                      << std::endl;
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
    constexpr const char* MESSAGE_OPTION = "message";
    router.optionsParser()->addOption<std::string>(MESSAGE_OPTION, "message for call smart contract");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION), DEFAULT_CONFIG_PATH};
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
        if(router.optionsParser()->hasOption(MESSAGE_OPTION)) {
            message_data = router.optionsParser()->getValue<std::string>(MESSAGE_OPTION);
        }
        else {
            message_data = helper.getValue<std::string>("messages", "compiled message to smart contract");
        }
        base::Bytes message = base::Bytes::fromHex(message_data);
        //====================================
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);

        auto[status, contract_response, gas_left] =
            client.transaction_to_contract(amount, bc::Address{from_address}, bc::Address{to_address}, base::Time::now(), gas, message, bc::Sign{});

        if(status) {
            std::cout << "Remote call of smart contract call success -> [" << status.getMessage()
                      << "], contract response[" << contract_response.toHex() << "], gas left[" << gas_left << "]"
                      << std::endl;
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

int compileCode(base::SubprogramRouter& router)
{
    constexpr const char* CODE_PATH_OPTION = "code_path";
    router.optionsParser()->addOption<std::string>(CODE_PATH_OPTION, "path to compiled code");
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        //====================================
        std::string code_file_path = router.optionsParser()->getValue<std::string>(CODE_PATH_OPTION);

        vm::Solc solc;

        try {
            auto contracts = solc.compile(code_file_path);
            if(!contracts) {
                std::cerr << "Compilation error\n";
                return base::config::EXIT_FAIL;
            }
            std::cout << "compiled contracts:" << std::endl;
            for(const auto& contract: contracts.value()) {
                std::cout << contract.getName() << std::endl;
            }
        }
        catch(const base::ParsingError& er) {
            std::cerr << er;
            return base::config::EXIT_FAIL;
        }
        catch(const base::SystemCallFailed& er) {
            std::cerr << er;
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
        std::cerr << "Error: " << er.what() << '\n';
        LOG_ERROR << "!exception in generate" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        std::cerr << "Unexpected error.\n";
        LOG_ERROR << "!unexpected exception caught in generate";
        return base::config::EXIT_FAIL;
    }
}


int generateKeys(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, DEFAULT_CONFIG_PATH, "config.json file path");
    constexpr const char* KEYS_OPTION = "path";
    constexpr const char* KEYS_OPTION_HINT = "directory in which a key pair will be generated";
    router.optionsParser()->addOption<std::string>(KEYS_OPTION, KEYS_OPTION_HINT);
    router.update();

    if(router.optionsParser()->hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        ParametersHelper helper{router.optionsParser()->getValue<std::string>(CONFIG_OPTION), DEFAULT_CONFIG_PATH};
        //====================================
        std::filesystem::path path;
        if(router.optionsParser()->hasOption(KEYS_OPTION)) {
            path = router.optionsParser()->getValue<std::string>(KEYS_OPTION);
        }
        else {
            path = helper.getValue<std::string>("keys", KEYS_OPTION_HINT);
        }
        //====================================

        if(!std::filesystem::exists(path)) {
            std::cerr << "Given path does not exist" << std::endl;
            return base::config::EXIT_FAIL;
        }
        else if(!std::filesystem::is_directory(path)) {
            std::cerr << "Given path is not a directory" << std::endl;
            return base::config::EXIT_FAIL;
        }

        LOG_INFO << "Generating key pair at " << path;
        std::cout << "Generating key pair at " << path << std::endl;
        const auto& [pub, priv] = base::generateKeys();

        auto public_path = base::config::makePublicKeyPath(path);
        if(std::filesystem::exists(public_path)) {
            std::cerr << "Error: " << public_path << " already exists.\n";
            LOG_ERROR << public_path << " file already exists";
            return base::config::EXIT_FAIL;
        }

        auto private_path = base::config::makePrivateKeyPath(path);
        if(std::filesystem::exists(private_path)) {
            std::cerr << "Error: " << private_path << " already exists.\n";
            LOG_ERROR << private_path << " file already exists";
            return base::config::EXIT_FAIL;
        }

        pub.save(public_path);
        std::cout << "Generated public key at " << public_path << std::endl;
        std::cout << "Address: " << bc::Address::fromPublicKey(pub) << std::endl;
        LOG_INFO << "Generated public key at " << public_path;

        priv.save(private_path);
        std::cout << "Generated private key at " << private_path << std::endl;
        LOG_INFO << "Generated private key at " << private_path;

        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Invalid arguments: " << router.helpMessage() << '\n';
        LOG_ERROR << "!exception in generate" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        std::cerr << "Error: " << er.what() << '\n';
        LOG_ERROR << "!exception in generate" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        std::cerr << "Unexpected error.\n";
        LOG_ERROR << "!unexpected exception caught in generate";
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
        std::cout << "Likelib2 client 0.1" << std::endl;
        return base::config::EXIT_OK;
    }

    return base::config::EXIT_OK;
}


int main(int argc, char** argv)
{
    try {
        base::initLog(base::Sink::FILE);
        base::SubprogramRouter router("client", mainProcess);
        router.addSubprogram("generate", "generate the pair of keys", generateKeys);
        router.addSubprogram("get_balance", "use for get balance from remote by account address", getBalance);
        router.addSubprogram("transfer", "use transfer balance from one address to another address", transfer);
        router.addSubprogram("test", "use test functions", testConnection);
        router.addSubprogram("push_contract", "load smart contract code to push to blockchain network", pushContract);
        router.addSubprogram("message_to_contract", "create message to call smart contract ", messageToContract);
        router.addSubprogram("compile", "compile smart contract ", compileCode);


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
