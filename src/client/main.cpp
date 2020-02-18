#include "parameters_helper.hpp"

#include "rpc/rpc.hpp"
#include "rpc/error.hpp"

#include "base/log.hpp"
#include "base/subprogram_router.hpp"
#include "base/error.hpp"
#include "base/hash.hpp"
#include "base/time.hpp"
#include "bc/transaction.hpp"


namespace
{
constexpr const char* const DEFAULT_CONFIG_PATH = "config.json";
constexpr const char* const KEY_FILE_PREFIX = "lkkey";
} // namespace


std::filesystem::path makePublicKeyPath(const std::filesystem::path& directory)
{
    auto ret = directory / KEY_FILE_PREFIX;
    ret.replace_extension(".pub");
    return ret;
}


std::filesystem::path makePrivateKeyPath(const std::filesystem::path& directory)
{
    return directory / KEY_FILE_PREFIX;
}


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
        bc::Address account_address{base::RsaPublicKey(base::Bytes::fromHex(account_address_str))};
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
        bc::Address to_address{base::RsaPublicKey(base::Bytes::fromHex(to_address_str))};
        //====================================
        bc::Balance amount;
        if(router.optionsParser()->hasOption(AMOUNT_OPTION)) {
            amount = router.optionsParser()->getValue<bc::Balance>(AMOUNT_OPTION);
        }
        else {
            amount = helper.getValue<bc::Balance>("amount", "transfer amount");
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
        auto public_key_path = makePublicKeyPath(keys_path);
        if(!std::filesystem::exists(public_key_path)) {
            std::cerr << "Error: public key file not found at " << public_key_path;
            LOG_ERROR << "error: public key not found by path " << public_key_path;
            return base::config::EXIT_FAIL;
        }
        auto public_key = base::RsaPublicKey::load(public_key_path);
        bc::Address from_address{std::move(public_key)};

        auto private_key_path = makePrivateKeyPath(keys_path);
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
        auto tx = std::move(txb).build();
        tx.sign(private_key);

        auto result = client.transaction(tx.getAmount(), tx.getFrom(), tx.getTo(), tx.getTimestamp(), *tx.getSign());
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

int test(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.optionsParser()->addOption<std::string>(CONFIG_OPTION, DEFAULT_CONFIG_PATH, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.optionsParser()->addOption<std::string>(HOST_OPTION, "address of host");
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

        auto data = base::Sha256::compute(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_REQUEST)).toHex();
        auto answer = client.test(data);
        LOG_INFO << "Received a response from rpc server " << host_address << ": " << answer;
        auto our_answer = base::Sha256::compute(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_RESPONSE)).toHex();

        if(answer == our_answer) {
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

        auto public_path = makePublicKeyPath(path);
        if(std::filesystem::exists(public_path)) {
            std::cerr << "Error: " << public_path << " already exists.\n";
            LOG_ERROR << public_path << " file already exists";
            return base::config::EXIT_FAIL;
        }

        auto private_path = makePrivateKeyPath(path);
        if(std::filesystem::exists(private_path)) {
            std::cerr << "Error: " << private_path << " already exists.\n";
            LOG_ERROR << private_path << " file already exists";
            return base::config::EXIT_FAIL;
        }

        pub.save(public_path);
        std::cout << "Generated public key at " << public_path << std::endl;
        std::cout << "Public key: " << pub;
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
        router.addSubprogram("test", "use test functions", test);

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
