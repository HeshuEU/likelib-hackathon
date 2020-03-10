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

namespace
{
constexpr const char* HOST_OPTION = "host";
constexpr const char* TO_ADDRESS_OPTION = "to";
constexpr const char* AMOUNT_OPTION = "amount";
constexpr const char* KEYS_DIRECTORY_OPTION = "keys";
constexpr const char* FEE_OPTION = "fee";
constexpr const char* ADDRESS_OPTION = "address";
constexpr const char* CODE_PATH_OPTION = "code";
constexpr const char* GAS_OPTION = "gas";
constexpr const char* INITIAL_MESSAGE_OPTION = "init";
constexpr const char* MESSAGE_OPTION = "message for call smart contract";

std::pair<base::RsaPublicKey, base::RsaPrivateKey> loadKeys(const std::filesystem::path& dir)
{
    auto public_key_path = base::config::makePublicKeyPath(dir);
    if(!std::filesystem::exists(public_key_path)) {
        RAISE_ERROR(base::InaccessibleFile, "cannot find public key file by path \"" + public_key_path.string() + '\"');
    }
    auto public_key = base::RsaPublicKey::load(public_key_path);

    auto private_key_path = base::config::makePrivateKeyPath(dir);
    if(!std::filesystem::exists(private_key_path)) {
        RAISE_ERROR(
            base::InaccessibleFile, "cannot find private key file by path \"" + private_key_path.string() + '\"');
    }
    auto private_key = base::RsaPrivateKey::load(private_key_path);

    return {std::move(public_key), std::move(private_key)};
}
} // namespace

//====================================

ActionBase::ActionBase(base::SubprogramRouter& router) : _router{router}
{}


int ActionBase::run()
{
    try {
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
    }
    catch(const base::ParsingError& er) {
        std::cerr << "Bad input arguments\n" << _router.helpMessage();
        LOG_ERROR << "[base::ParsingError caught during execution of Client::" << getName() << "] " << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cerr << "RPC error. " << er.what() << "\n";
        LOG_ERROR << "[rpc::RpcError caught during client::" << getName() << "] " << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const base::Error& er) {
        std::cerr << "Unexpected error." << er.what() << "\n";
        LOG_ERROR << "[base::Error caught during Client::" << getName() << "] " << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const std::exception& er) {
        std::cerr << "Unexpected error." << er.what() << "\n";
        LOG_ERROR << "[std::exception caught during Client::" << getName() << "] " << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        std::cerr << "Unexpected error.\n";
        LOG_ERROR << "[exception of an unknown type caught during client::" << getName() << "] ";
        return base::config::EXIT_FAIL;
    }

    return base::config::EXIT_OK;
}

//====================================

ActionTransfer::ActionTransfer(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionTransfer::getName() const
{
    static const std::string_view name = "Transfer";
    return name;
}


void ActionTransfer::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(HOST_OPTION, "address of host");
    parser.addRequiredOption<std::string>(TO_ADDRESS_OPTION, "address of recipient account");
    parser.addRequiredOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    parser.addRequiredOption<std::string>(KEYS_DIRECTORY_OPTION, "path to a directory with keys");
    parser.addRequiredOption<bc::Balance>(FEE_OPTION, "fee count");
}


int ActionTransfer::loadOptions(const base::ProgramOptionsParser& parser)
{
    _host_address = parser.getValue<std::string>(HOST_OPTION);
    _to_address = bc::Address{parser.getValue<std::string>(TO_ADDRESS_OPTION)};
    _amount = parser.getValue<bc::Balance>(AMOUNT_OPTION);
    _fee = parser.getValue<bc::Balance>(FEE_OPTION);
    _keys_dir = parser.getValue<std::string>(KEYS_DIRECTORY_OPTION);
    return base::config::EXIT_OK;
}


int ActionTransfer::execute()
{
    auto [pub, priv] = loadKeys(_keys_dir);
    auto from_address = bc::Address(pub);

    rpc::RpcClient client(_host_address);

    LOG_INFO << "Transfer from " << from_address << " to " << _to_address << " with amount " << _amount
             << " to rpc server " << _host_address;
    LOG_INFO << "Trying to connect to rpc a server at " << _host_address;

    bc::TransactionBuilder txb;
    txb.setType(bc::Transaction::Type::MESSAGE_CALL);
    txb.setFrom(std::move(from_address));
    txb.setTo(std::move(_to_address));
    txb.setAmount(_amount);
    txb.setTimestamp(base::Time::now());
    txb.setFee(_fee);
    txb.setData({});
    auto tx = std::move(txb).build();

    tx.sign(pub, priv);

    auto [status, result, gas_left] = client.transaction_message_call(
        tx.getAmount(), tx.getFrom(), tx.getTo(), tx.getTimestamp(), tx.getFee(), "", tx.getSign());

    if(status) {
        std::cout << "Transaction successfully performed";
    }
    else {
        std::cerr << "Transaction failed with message: " << status.getMessage() << std::endl;
    }

    LOG_INFO << "Remote call of transfer -> [" << result << "]";
    return base::config::EXIT_OK;
}

//====================================

ActionGetBalance::ActionGetBalance(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionGetBalance::getName() const
{
    static const std::string_view name = "GetBalance";
    return name;
}


void ActionGetBalance::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(HOST_OPTION, "address of host");
    parser.addRequiredOption<std::string>(ADDRESS_OPTION, "address of target account");
}


int ActionGetBalance::loadOptions(const base::ProgramOptionsParser& parser)
{
    _host_address = parser.getValue<std::string>(HOST_OPTION);
    _account_address = bc::Address{parser.getValue<std::string>(ADDRESS_OPTION)};
    return base::config::EXIT_OK;
}


int ActionGetBalance::execute()
{
    LOG_INFO << "GetBalance for address " << _account_address;
    LOG_INFO << "Trying to connect to rpc server at " << _host_address;
    rpc::RpcClient client(_host_address);
    auto result = client.balance(_account_address);
    std::cout << "balance of " << _account_address << " is " << result << std::endl;
    LOG_INFO << "Remote call of getBalance(" << _account_address << ") -> [" << result << "]";
    return base::config::EXIT_OK;
}

//====================================

ActionTestConnection::ActionTestConnection(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionTestConnection::getName() const
{
    static const std::string_view name = "TestConnection";
    return name;
}


void ActionTestConnection::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(HOST_OPTION, "address of host");
}


int ActionTestConnection::loadOptions(const base::ProgramOptionsParser& parser)
{
    _host_address = parser.getValue<std::string>(HOST_OPTION);
    return base::config::EXIT_OK;
}


int ActionTestConnection::execute()
{
    LOG_INFO << "Test connect to rpc server by: " << _host_address;
    LOG_INFO << "Trying to connect to rpc server by: " << _host_address;
    rpc::RpcClient client(_host_address);

    auto answer = client.test(config::API_VERSION);

    if(answer) {
        std::cout << "Test passed" << std::endl;
        LOG_INFO << "Test passed";
    }
    else {
        std::cout << "Test failed" << std::endl;
        LOG_INFO << "Test failed with message: " << answer.getMessage();
    }
    return base::config::EXIT_OK;
}

//====================================

ActionCreateContract::ActionCreateContract(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionCreateContract::getName() const
{
    static const std::string_view name = "CreateContract";
    return name;
}


void ActionCreateContract::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(HOST_OPTION, "address of host");
    parser.addRequiredOption<std::string>(CODE_PATH_OPTION, "path to compiled code");
    parser.addRequiredOption<bc::Balance>(AMOUNT_OPTION, "amount of Lk to transfer");
    parser.addRequiredOption<bc::Balance>(GAS_OPTION, "gas count");
    parser.addRequiredOption<std::string>(KEYS_DIRECTORY_OPTION, "path to a directory with keys");
    parser.addOption<std::string>(INITIAL_MESSAGE_OPTION, "message for initialize smart contract");
}


int ActionCreateContract::loadOptions(const base::ProgramOptionsParser& parser)
{
    _host_address = parser.getValue<std::string>(HOST_OPTION);

    std::filesystem::path code_file_path = parser.getValue<std::string>(CODE_PATH_OPTION);
    auto compiled_code = base::readConfig(code_file_path);
    _compiled_contract = compiled_code.get<std::string>("object");

    _keys_dir = parser.getValue<std::string>(KEYS_DIRECTORY_OPTION);

    _amount = parser.getValue<bc::Balance>(AMOUNT_OPTION);
    _gas = parser.getValue<bc::Balance>(GAS_OPTION);

    if(parser.hasOption(INITIAL_MESSAGE_OPTION)) {
        _message = parser.getValue<std::string>(INITIAL_MESSAGE_OPTION);
    }

    return base::config::EXIT_OK;
}


int ActionCreateContract::execute()
{
    auto [pub, priv] = loadKeys(_keys_dir);
    auto from_address = bc::Address(pub);

    bc::TransactionBuilder txb;
    txb.setType(bc::Transaction::Type::CONTRACT_CREATION);
    txb.setAmount(_amount);
    txb.setFrom(std::move(from_address));
    txb.setTo(bc::Address::null());
    txb.setTimestamp(base::Time::now());
    txb.setFee(_gas);

    bc::ContractInitData init_data{base::Bytes::fromHex(_compiled_contract), base::Bytes::fromHex(_message)};
    txb.setData(base::toBytes(init_data));

    auto tx = std::move(txb).build();
    tx.sign(pub, priv);

    std::cout << tx << std::endl;

    LOG_INFO << "Trying to connect to rpc server by: " << _host_address;
    rpc::RpcClient client(_host_address);
    auto [status, contract_address, gas_left] = client.transaction_create_contract(
        tx.getAmount(), tx.getFrom(), tx.getTimestamp(), tx.getFee(), _compiled_contract, _message, tx.getSign());

    if(status) {
        std::cout << "Remote call of creation smart contract success -> [" << status.getMessage()
                  << "], contract created at [" << contract_address.toString() << "], gas left[" << gas_left << "]"
                  << std::endl;
        return base::config::EXIT_OK;
    }
    else {
        std::cout << "Remote call of creation smart contract is failed -> [" << status.getMessage() << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }

    return base::config::EXIT_OK;
}


//====================================

ActionMessageCall::ActionMessageCall(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionMessageCall::getName() const
{
    static const std::string_view name = "MessageCall";
    return name;
}


void ActionMessageCall::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(HOST_OPTION, "address of host");
    parser.addRequiredOption<std::string>(TO_ADDRESS_OPTION, "address of \"to\" contract");
    parser.addRequiredOption<bc::Balance>(AMOUNT_OPTION, "amount count");
    parser.addRequiredOption<bc::Balance>(GAS_OPTION, "gas count");
    parser.addRequiredOption<std::string>(KEYS_DIRECTORY_OPTION, "path to a directory with keys");
    parser.addRequiredOption<std::string>(MESSAGE_OPTION, "message for call smart contract");
}


int ActionMessageCall::loadOptions(const base::ProgramOptionsParser& parser)
{
    _host_address = parser.getValue<std::string>(HOST_OPTION);
    _to_address = bc::Address{parser.getValue<std::string>(TO_ADDRESS_OPTION)};
    _amount = parser.getValue<bc::Balance>(AMOUNT_OPTION);
    _gas = parser.getValue<bc::Balance>(GAS_OPTION);
    _keys_dir = parser.getValue<std::string>(KEYS_DIRECTORY_OPTION);
    _message = parser.getValue<std::string>(MESSAGE_OPTION);
    return base::config::EXIT_OK;
}


int ActionMessageCall::execute()
{
    auto [pub, priv] = loadKeys(_keys_dir);
    auto from_address = bc::Address(pub);

    bc::TransactionBuilder txb;
    txb.setType(bc::Transaction::Type::MESSAGE_CALL);
    txb.setAmount(_amount);
    txb.setFrom(std::move(from_address));
    txb.setTo(std::move(_to_address));
    txb.setTimestamp(base::Time::now());
    txb.setFee(_gas);
    txb.setData(base::Bytes::fromHex(_message));

    auto tx = std::move(txb).build();
    tx.sign(pub, priv);

    LOG_INFO << "Try to connect to rpc server by: " << _host_address;
    rpc::RpcClient client(_host_address);

    auto [status, contract_response, gas_left] = client.transaction_message_call(
        tx.getAmount(), tx.getFrom(), tx.getTo(), tx.getTimestamp(), tx.getFee(), _message, tx.getSign());

    if(status) {
        std::cout << "Remote call of smart contract call success -> [" << status.getMessage() << "], contract response["
                  << contract_response << "], gas left[" << gas_left << "]" << std::endl;
        return base::config::EXIT_OK;
    }
    else {
        std::cout << "Remote call of smart contract call is failed -> [" << status.getMessage() << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }

    return base::config::EXIT_OK;
}

//====================================

ActionCompile::ActionCompile(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionCompile::getName() const
{
    static const std::string_view name = "Compile";
    return name;
}


void ActionCompile::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(CODE_PATH_OPTION, "path to a Solidity code");
}


int ActionCompile::loadOptions(const base::ProgramOptionsParser& parser)
{
    _code_file_path = parser.getValue<std::string>(CODE_PATH_OPTION);
    return base::config::EXIT_OK;
}


int ActionCompile::execute()
{
    vm::Solc compiler;

    try {
        auto contracts = compiler.compile(_code_file_path);
        if(!contracts) {
            std::cerr << "Compilation error\n";
            return base::config::EXIT_FAIL;
        }
        std::cout << "Compiled contracts:" << std::endl;
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

//====================================

ActionGenerateKeys::ActionGenerateKeys(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionGenerateKeys::getName() const
{
    static const std::string_view name = "GenerateKeys";
    return name;
}


void ActionGenerateKeys::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(KEYS_DIRECTORY_OPTION, "directory in which a key pair will be generated");
}


int ActionGenerateKeys::loadOptions(const base::ProgramOptionsParser& parser)
{
    _keys_dir = parser.getValue<std::string>(KEYS_DIRECTORY_OPTION);

    if(!std::filesystem::exists(_keys_dir)) {
        std::cerr << "Given path does not exist" << std::endl;
        return base::config::EXIT_FAIL;
    }
    else if(!std::filesystem::is_directory(_keys_dir)) {
        std::cerr << "Given path is not a directory" << std::endl;
        return base::config::EXIT_FAIL;
    }

    return base::config::EXIT_OK;
}


int ActionGenerateKeys::execute()
{
    LOG_INFO << "Generating key pair at " << _keys_dir;
    std::cout << "Generating key pair at " << _keys_dir << std::endl;
    const auto& [pub, priv] = base::generateKeys();

    auto public_path = base::config::makePublicKeyPath(_keys_dir);
    if(std::filesystem::exists(public_path)) {
        std::cerr << "Error: " << public_path << " already exists.\n";
        LOG_ERROR << public_path << " file already exists";
        return base::config::EXIT_FAIL;
    }

    auto private_path = base::config::makePrivateKeyPath(_keys_dir);
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

//====================================

ActionKeysInfo::ActionKeysInfo(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionKeysInfo::getName() const
{
    static const std::string_view name = "KeysInfo";
    return name;
}


void ActionKeysInfo::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(KEYS_DIRECTORY_OPTION, "directory with a key pair");
}


int ActionKeysInfo::loadOptions(const base::ProgramOptionsParser& parser)
{
    _keys_dir = parser.getValue<std::string>(KEYS_DIRECTORY_OPTION);

    if(!std::filesystem::exists(_keys_dir)) {
        std::cerr << "Given path does not exist" << std::endl;
        return base::config::EXIT_FAIL;
    }
    else if(!std::filesystem::is_directory(_keys_dir)) {
        std::cerr << "Given path is not a directory" << std::endl;
        return base::config::EXIT_FAIL;
    }

    return base::config::EXIT_OK;
}


int ActionKeysInfo::execute()
{
    auto public_path = base::config::makePublicKeyPath(_keys_dir);
    if(!std::filesystem::exists(public_path)) {
        std::cerr << "Error: " << public_path << " doesn't exist.\n";
        LOG_ERROR << public_path << " file not exists";
        return base::config::EXIT_FAIL;
    }

    auto private_path = base::config::makePrivateKeyPath(_keys_dir);
    if(!std::filesystem::exists(private_path)) {
        std::cerr << "Error: " << private_path << " doesn't exist.\n";
        LOG_ERROR << private_path << " file not exists";
        return base::config::EXIT_FAIL;
    }

    auto pub = base::RsaPublicKey::load(public_path);
    auto priv = base::RsaPrivateKey::load(private_path);

    std::cout << "Address: " << bc::Address(pub) << std::endl;
    std::cout << "Sha256 hash of public key: " << base::Sha256::compute(pub.toBytes()) << std::endl;
    std::cout << "Sha256 hash of private key: " << base::Sha256::compute(priv.toBytes()) << std::endl;

    return base::config::EXIT_OK;
}


//====================================

ActionInfo::ActionInfo(base::SubprogramRouter& router) : ActionBase{router}
{}


const std::string_view& ActionInfo::getName() const
{
    static const std::string_view name = "Info";
    return name;
}


void ActionInfo::setupOptionsParser(base::ProgramOptionsParser& parser)
{
    parser.addRequiredOption<std::string>(HOST_OPTION, "address of host");
}


int ActionInfo::loadOptions(const base::ProgramOptionsParser& parser)
{
    _host_address = parser.getValue<std::string>(HOST_OPTION);
    return base::config::EXIT_OK;
}


int ActionInfo::execute()
{
    LOG_INFO << "Trying to connect to rpc server at " << _host_address;
    rpc::RpcClient client(_host_address);
    auto result = client.info();
    std::cout << "Top block hash: " << result.top_block_hash << std::endl;
    LOG_INFO << "Remote call of Info: " << result.top_block_hash;
    return base::config::EXIT_OK;
}