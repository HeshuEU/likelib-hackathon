#include "rpc/rpc.hpp"
#include "rpc/error.hpp"

#include "base/log.hpp"
#include "base/program_options.hpp"
#include "base/error.hpp"
#include "base/hash.hpp"

namespace
{

static constexpr const char* HOST_ADDRESS_OPTION = "host";
static constexpr const char* HOST_ADDRESS_FLAG = "host";

static constexpr const char* MONEY_OPTION = "money,m";
static constexpr const char* MONEY_FLAG = "money";

static constexpr const char* ACCOUNT_ADDRESS_OPTION = "address,a";
static constexpr const char* ACCOUNT_ADDRESS_FLAG = "address";

static constexpr const char* FROM_ACCOUNT_ADDRESS_OPTION = "from_address,f";
static constexpr const char* FROM_ACCOUNT_ADDRESS_FLAG = "from_address";

static constexpr const char* TO_ACCOUNT_ADDRESS_OPTION = "to_address,t";
static constexpr const char* TO_ACCOUNT_ADDRESS_FLAG = "to_address";

static constexpr const char* PROGRAM_NAME = "rpc-client";


void createKey(const base::ProgramOptionsParser& parser)
{
    // local processing
}

int getBalance(const base::ProgramOptionsParser& parser)
{
    if(parser.hasOption("help")) {
        std::cout << parser.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }
    if(!parser.hasOption(HOST_ADDRESS_FLAG)) {
        std::cout << "No option required option[" << HOST_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }
    if(!parser.hasOption(ACCOUNT_ADDRESS_FLAG)) {
        std::cout << "No option required option[" << ACCOUNT_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }

    try {
        auto host = parser.getValue<std::string>(HOST_ADDRESS_FLAG);
        auto address = parser.getValue<std::string>(ACCOUNT_ADDRESS_FLAG);

        LOG_INFO << "Try to connect to rpc server by: " << host;
        rpc::RpcClient client(host);
        auto result = client.balance(address.c_str());
        std::cout << "Remote call of get_balance -> [" << result << "]" << std::endl;
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cout << "Bad input arguments" << std::endl;
        LOG_ERROR << "[exception in getBalance]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cout << "RPC error" << std::endl;
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

int transaction(const base::ProgramOptionsParser& parser)
{
    if(parser.hasOption("help")) {
        std::cout << parser.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }
    if(!parser.hasOption(HOST_ADDRESS_FLAG)) {
        std::cout << "No option required option[" << HOST_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }
    if(!parser.hasOption(FROM_ACCOUNT_ADDRESS_FLAG)) {
        std::cout << "No option required option[" << FROM_ACCOUNT_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }
    if(!parser.hasOption(TO_ACCOUNT_ADDRESS_FLAG)) {
        std::cout << "No option required option[" << TO_ACCOUNT_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }
    if(!parser.hasOption(MONEY_FLAG)) {
        std::cout << "No option required option[" << MONEY_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }

    try {
        auto host = parser.getValue<std::string>(HOST_ADDRESS_FLAG);
        auto from_address = parser.getValue<std::string>(FROM_ACCOUNT_ADDRESS_FLAG);
        auto to_address = parser.getValue<std::string>(TO_ACCOUNT_ADDRESS_FLAG);
        auto amount = parser.getValue<std::uint32_t>(MONEY_FLAG);

        rpc::RpcClient client(host);
        LOG_INFO << "Try to connect to rpc server by: " << host;
        auto result = client.transaction(amount, from_address.c_str(), to_address.c_str());
        std::cout << "Remote call of transaction -> [" << result << "]" << std::endl;
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cout << "Bad input arguments" << std::endl;
        LOG_ERROR << "[exception in transaction]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cout << "RPC error" << std::endl;
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

int test(const base::ProgramOptionsParser& parser)
{
    if(parser.hasOption("help")) {
        std::cout << parser.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }
    if(!parser.hasOption(HOST_ADDRESS_FLAG)) {
        std::cout << "No option required option[" << HOST_ADDRESS_OPTION << "]" << std::endl;
        return base::config::EXIT_FAIL;
    }

    try {
        auto host = parser.getValue<std::string>("host");

        //    auto data = base::sha256(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_REQUEST));
        //    auto answer = client.test(data.toString());
        //    auto our_ansver = base::sha256(base::Bytes(base::config::RPC_CURRENT_SECRET_TEST_RESPONSE)).toString();

        LOG_INFO << "Try to connect to rpc server by: " << host;
        rpc::RpcClient client(host);

        auto answer = client.test(base::config::RPC_CURRENT_SECRET_TEST_REQUEST);
        auto our_ansver = std::string(base::config::RPC_CURRENT_SECRET_TEST_RESPONSE);

        if(answer == our_ansver) {
            std::cout << "Test passed" << std::endl;
        }
        else {
            std::cout << "Test failed" << std::endl;
        }
        return base::config::EXIT_OK;
    }
    catch(const base::ParsingError& er) {
        std::cout << "Bad input arguments" << std::endl;
        LOG_ERROR << "[exception in test]" << er.what();
        return base::config::EXIT_FAIL;
    }
    catch(const rpc::RpcError& er) {
        std::cout << "RPC error" << std::endl;
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

} // namespace

int main(int argc, char** argv)
{
    try {
        base::initLog(base::LogLevel::ALL, base::Sink::FILE);

        base::ProgramOptionsParser parser(PROGRAM_NAME);
        parser.addFlag("version,v", "Print version of program and version of rpc protocol");

        auto get_balance_sub_parser =
            parser.createSubParser("get_balance", "use for get balance from remote by account address", getBalance);
        get_balance_sub_parser->addOption<std::string>(HOST_ADDRESS_OPTION,
                                                       "address of remote host in format: \"<ip>:<port>\"");
        get_balance_sub_parser->addOption<std::string>(ACCOUNT_ADDRESS_OPTION, "account address");

        auto transaction_sub_parser =
            parser.createSubParser("transaction", "use for get balance from remote by account address", transaction);
        transaction_sub_parser->addOption<std::string>(HOST_ADDRESS_OPTION,
                                                       "address of remote host in format: \"<ip>:<port>\"");
        transaction_sub_parser->addOption<std::string>(FROM_ACCOUNT_ADDRESS_OPTION, "from account address");
        transaction_sub_parser->addOption<std::string>(TO_ACCOUNT_ADDRESS_OPTION, "to account address");
        transaction_sub_parser->addOption<std::uint32_t>(MONEY_OPTION, "transaction money");

        auto test_sub_parser = parser.createSubParser("test", "use test functions", test);
        test_sub_parser->addOption<std::string>(HOST_ADDRESS_OPTION,
                                                "address of remote host in format: \"<ip>:<port>\"");

        try{
            auto exit_code = parser.process(argc, argv);
            if(exit_code != base::config::EXIT_OK) {
                return exit_code;
            }
        }
        catch (const base::InvalidArgument& error){
            std::cout << "Command is now exist. Run pc-client --help to see allowed commands and options" << std::endl;
        }
        catch (const base::ParsingError& error){
            std::cout << "Failed to parse command options. Run " << PROGRAM_NAME << " --help to see allowed commands and options" << std::endl;
        }

        if(parser.hasOption("help")) {
            std::cout << parser.helpMessage() << std::endl;
            return base::config::EXIT_OK;
        }

        if(parser.hasOption("version")) {
            std::cout << "application version: 0.1 - alpha" << std::endl;
            std::cout << "rpc protocol version: 0.1 - alpha" << std::endl;
            return base::config::EXIT_OK;
        }
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
