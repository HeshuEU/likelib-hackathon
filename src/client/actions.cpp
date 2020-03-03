#include "actions.hpp"
#include "config.hpp"

#include "base/config.hpp"
#include "base/hash.hpp"
#include "base/log.hpp"
#include "base/property_tree.hpp"
#include "base/subprogram_router.hpp"
#include "base/time.hpp"
#include "bc/transaction.hpp"
#include "rpc/rpc.hpp"
#include "rpc/error.hpp"
#include "vm/messages.hpp"

#include <filesystem>
#include <iostream>
#include <string>


//====================================

ActionBase::ActionBase(base::SubprogramRouter& router) : _router{router}
{}


int ActionBase::run()
{
    setupOptionsParser(_router.getOptionsParser());
    _router.update();

    if(_router.getOptionsParser().hasOption("help")) {
        std::cout << _router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if(auto ret = loadOptions(_router.getOptionsParser()); ret != base::config::EXIT_OK) {
        return ret;
    }
    if(auto ret = execute(); ret != base::config::EXIT_OK) {
        return ret;
    }

    return base::config::EXIT_OK;
}

//====================================

void ActionTransfer::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addOption<std::string>(CONFIG_OPTION, "rpc_config json file path");
    parser.addOption<std::string>(HOST_OPTION, "address of host");
    parser.addOption<std::string>(TO_ADDRESS_OPTION, "address of recipient account");
    parser.addOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    parser.addOption<std::string>(KEYS_DIRECTORY_OPTION, "path to a directory with keys");
    parser.addOption<bc::Balance>(FEE_OPTION, "fee count");
}


int ActionTransfer::loadOptions(const base::ProgramOptionsParser& parser) {
    std::filesystem::path config_file_path = parser.getValue<std::string>(CONFIG_OPTION);

    //====================================
    if(parser.hasOption(HOST_OPTION)) {
        _host_address = parser.getValue<std::string>(HOST_OPTION);
    }
    //====================================
    std::string to_address_str;
    if(parser.hasOption(TO_ADDRESS_OPTION)) {
        to_address_str = parser.getValue<std::string>(TO_ADDRESS_OPTION);
    }
    _to_address = bc::Address{to_address_str};
    //====================================
    if(parser.hasOption(AMOUNT_OPTION)) {
        _amount = parser.getValue<bc::Balance>(AMOUNT_OPTION);
    }
    //====================================
    if(parser.hasOption(FEE_OPTION)) {
        _fee = parser.getValue<bc::Balance>(FEE_OPTION);
    }
    //====================================
    if(parser.hasOption(KEYS_DIRECTORY_OPTION)) {
        _keys_path = parser.getValue<std::string>(KEYS_DIRECTORY_OPTION);
    }
    //====================================
    auto public_key_path = base::config::makePublicKeyPath(_keys_path);
    if(!std::filesystem::exists(public_key_path)) {
        std::cerr << "Error: public key file not found at " << public_key_path;
        LOG_ERROR << "error: public key not found by path " << public_key_path;
        return base::config::EXIT_FAIL;
    }
    _public_key = base::RsaPublicKey::load(public_key_path);
    _from_address = bc::Address(*_public_key);

    auto private_key_path = base::config::makePrivateKeyPath(_keys_path);
    if(!std::filesystem::exists(private_key_path)) {
        std::cerr << "Error: private key file not found at " << private_key_path;
        LOG_ERROR << "error: private key not found by path " << private_key_path;
        return base::config::EXIT_FAIL;
    }
    _private_key = base::RsaPrivateKey::load(private_key_path);

    return base::config::EXIT_OK;
}


int ActionTransfer::execute()
{
    try {
        rpc::RpcClient client(_host_address);
        LOG_INFO << "Transfer from " << _from_address << " to " << _to_address << " with amount " << _amount
            << " to rpc server " << _host_address;
        LOG_INFO << "Try to connect to rpc a server at " << _host_address;
        bc::TransactionBuilder txb;
        txb.setFrom(std::move(_from_address));
        txb.setTo(std::move(_to_address));
        txb.setAmount(_amount);
        txb.setTimestamp(base::Time::now());
        txb.setFee(_fee);
        auto tx = std::move(txb).build();
        tx.sign(*_public_key, *_private_key);
        auto [status, result, gas_left] = client.transaction_message_call(
                tx.getAmount(), tx.getFrom(), tx.getTo(), tx.getTimestamp(), tx.getFee(), "", tx.getSign());
        std::cout << result << std::endl;
        LOG_INFO << "Remote call of transaction -> [" << result << "]";
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments.\n" << _router.helpMessage();
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


//====================================

void ActionGetBalance::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addOption<std::string>(CONFIG_OPTION, "rpc_config json file path");
    parser.addOption<std::string>(HOST_OPTION, "address of host");
    parser.addOption<std::string>(ADDRESS_OPTION, "address of target account");
}


int ActionGetBalance::loadOptions(const base::ProgramOptionsParser& parser)
{
    std::string host_address;
    if(parser.hasOption(HOST_OPTION)) {
        host_address = parser.getValue<std::string>(HOST_OPTION);
    }
    //====================================
    std::string account_address_str;
    if(parser.hasOption(ADDRESS_OPTION)) {
        account_address_str = parser.getValue<std::string>(ADDRESS_OPTION);
    }
    _account_address = bc::Address{account_address_str};
}


int ActionGetBalance::execute()
{
    try {
        LOG_INFO << "GetBalance for address " << _account_address;
        LOG_INFO << "Trying to connect to rpc server at " << _host_address;
        rpc::RpcClient client(_host_address);
        auto result = client.balance(_account_address);
        std::cout << "balance of " << _account_address << " is " << result << std::endl;
        LOG_INFO << "Remote call of getBalance(" << _account_address << ") -> [" << result << "]";
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments.\n" << _router.helpMessage();
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

//====================================

void ActionTestConnection::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addOption<std::string>(HOST_OPTION, "address of host");
}


int ActionTestConnection::loadOptions(const base::ProgramOptionsParser& parser)
{
    std::string host_address;
    if(_router.getOptionsParser().hasOption(HOST_OPTION)) {
        host_address = parser.getValue<std::string>(HOST_OPTION);
    }
}


int ActionTestConnection::execute()
{
    try {
        LOG_INFO << "Test connect to rpc server by: " << _host_address;
        LOG_INFO << "Try to connect to rpc server by: " << _host_address;
        rpc::RpcClient client(_host_address);

        auto answer = client.test(config::API_VERSION);

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
        std::cerr << "Bad input arguments\n" << _router.helpMessage();
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

//====================================

void ActionCreateContract::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addOption<std::string>(HOST_OPTION, "address of host");
    parser.addOption<std::string>(CODE_PATH_OPTION, "path to compiled code");
    parser.addOption<std::string>(FROM_ADDRESS_OPTION, "address of \"from\" account");
    parser.addOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    parser.addOption<bc::Balance>(GAS_OPTION, "gas count");
    parser.addOption<std::string>(INITIAL_MESSAGE_OPTION, "message for initialize smart contract");
}


int ActionCreateContract::loadOptions(const base::ProgramOptionsParser& parser)
{
    if(parser.hasOption(HOST_OPTION)) {
        _host_address = parser.getValue<std::string>(HOST_OPTION);
    }
    //====================================
    std::string file_path;
    if(parser.hasOption(CODE_PATH_OPTION)) {
        file_path = parser.getValue<std::string>(CODE_PATH_OPTION);
    }
    else {
        if(file_path.empty()) {
            std::cout << "Path to code not set up" << std::endl;
            return base::config::EXIT_FAIL;
        }
    }
    std::string contract;
    try {
        auto compiled_code = base::readConfig(file_path);
        contract = compiled_code.get<std::string>("object");
    }
    catch(const base::InaccessibleFile& er) {
        std::cout << "File not found by path: " << file_path << std::endl;
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
    if(parser.hasOption(FROM_ADDRESS_OPTION)) {
        from_address = parser.getValue<std::string>(FROM_ADDRESS_OPTION);
    }
    //====================================
    bc::Balance amount;
    if(parser.hasOption(AMOUNT_OPTION)) {
        amount = parser.getValue<bc::Balance>(AMOUNT_OPTION);
    }
    //====================================
    bc::Balance gas;
    if(parser.hasOption(GAS_OPTION)) {
        gas = parser.getValue<bc::Balance>(GAS_OPTION);
    }
    //====================================
    std::string message;
    if(parser.hasOption(INITIAL_MESSAGE_OPTION)) {
        message = parser.getValue<std::string>(INITIAL_MESSAGE_OPTION);
    }
    else {
        message = "";//helper.getValue<std::string>("messages", "message for initialization smart contract");
    }
    //====================================

    return base::config::EXIT_OK;
}


int ActionCreateContract::execute() {
    try {
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);

        auto [status, contract_address, gas_left] = client.transaction_create_contract(
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


int messageCall(base::SubprogramRouter& router)
{
    constexpr const char* CONFIG_OPTION = "config";
    router.getOptionsParser().addOption<std::string>(CONFIG_OPTION, "rpc_config json file path");
    constexpr const char* HOST_OPTION = "host";
    router.getOptionsParser().addOption<std::string>(HOST_OPTION, "address of host");
    constexpr const char* FROM_ADDRESS_OPTION = "from";
    router.getOptionsParser().addOption<std::string>(FROM_ADDRESS_OPTION, "address of \"from\" account");
    constexpr const char* TO_ADDRESS_OPTION = "to";
    router.getOptionsParser().addOption<std::string>(TO_ADDRESS_OPTION, "address of \"to\" contract");
    constexpr const char* AMOUNT_OPTION = "amount";
    router.getOptionsParser().addOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    constexpr const char* GAS_OPTION = "gas";
    router.getOptionsParser().addOption<bc::Balance>(GAS_OPTION, "gas count");
    constexpr const char* MESSAGE_OPTION = "message";
    router.getOptionsParser().addOption<std::string>(MESSAGE_OPTION, "message for call smart contract");
    router.update();

    if(router.getOptionsParser().hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        //====================================
        std::string host_address;
        if(router.getOptionsParser().hasOption(HOST_OPTION)) {
            host_address = router.getOptionsParser().getValue<std::string>(HOST_OPTION);
        }
        //====================================
        std::string from_address;
        if(router.getOptionsParser().hasOption(FROM_ADDRESS_OPTION)) {
            from_address = router.getOptionsParser().getValue<std::string>(FROM_ADDRESS_OPTION);
        }
        //====================================
        std::string to_address;
        if(router.getOptionsParser().hasOption(TO_ADDRESS_OPTION)) {
            to_address = router.getOptionsParser().getValue<std::string>(TO_ADDRESS_OPTION);
        }
        //====================================
        bc::Balance amount;
        if(router.getOptionsParser().hasOption(AMOUNT_OPTION)) {
            amount = router.getOptionsParser().getValue<bc::Balance>(AMOUNT_OPTION);
        }
        //====================================
        bc::Balance gas;
        if(router.getOptionsParser().hasOption(GAS_OPTION)) {
            gas = router.getOptionsParser().getValue<bc::Balance>(GAS_OPTION);
        }
        //====================================
        std::string message;
        if(router.getOptionsParser().hasOption(MESSAGE_OPTION)) {
            message = router.getOptionsParser().getValue<std::string>(MESSAGE_OPTION);
        }
        //====================================
        LOG_INFO << "Try to connect to rpc server by: " << host_address;
        rpc::RpcClient client(host_address);

        auto [status, contract_response, gas_left] = client.transaction_message_call(
                amount, bc::Address{from_address}, bc::Address{to_address}, base::Time::now(), gas, message, bc::Sign{});

        if(status) {
            std::cout << "Remote call of smart contract call success -> [" << status.getMessage()
                      << "], contract response[" << contract_response << "], gas left[" << gas_left << "]" << std::endl;
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
    router.getOptionsParser().addOption<std::string>(CODE_PATH_OPTION, "path to compiled code");
    router.update();

    if(router.getOptionsParser().hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        //====================================
        auto code_file_path = router.getOptionsParser().getValue<std::string>(CODE_PATH_OPTION);

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
    router.getOptionsParser().addOption<std::string>(CONFIG_OPTION, "config.json file path");
    constexpr const char* KEYS_OPTION = "path";
    constexpr const char* KEYS_OPTION_HINT = "directory in which a key pair will be generated";
    router.getOptionsParser().addOption<std::string>(KEYS_OPTION, KEYS_OPTION_HINT);
    router.update();

    if(router.getOptionsParser().hasOption("help")) {
        std::cout << router.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    try {
        //====================================
        std::filesystem::path path;
        if(router.getOptionsParser().hasOption(KEYS_OPTION)) {
            path = router.getOptionsParser().getValue<std::string>(KEYS_OPTION);
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
        std::cout << "Address: " << bc::Address(pub) << std::endl;
        std::cout << "Hash of public key: " << base::Sha256::compute(pub.toBytes()) << std::endl;
        std::cout << "Hash of private key: " << base::Sha256::compute(priv.toBytes()) << std::endl;
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
