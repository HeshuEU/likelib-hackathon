#include "rpc/rpc.hpp"

#include "base/log.hpp"
#include "base/program_options.hpp"
#include "base/error.hpp"

namespace
{
void printHelp(const base::ProgramOptionsParser& parser)
{}

void createKey(const base::ProgramOptionsParser& parser)
{
    // local processing
}

void getBalance(const base::ProgramOptionsParser& parser)
{
    if(parser.hasOption("help")) {
        std::cout << parser.helpMessage() << std::endl;
        return;
    }
    auto host = parser.getValue<std::string>("host");
    LOG_INFO << "Try to connect to rpc server by: " << host;
    rpc::RpcClient client(host);

    auto address = parser.getValue<std::string>("address");
    auto result = client.balance(address.c_str());
    std::cout << "Remote call of balance [" << result << "]" << std::endl;
}

void transaction(const base::ProgramOptionsParser& parser)
{
    if(parser.hasOption("help")) {
        std::cout << parser.helpMessage() << std::endl;
        return;
    }
    auto host = parser.getValue<std::string>("host");
    LOG_INFO << "Try to connect to rpc server by: " << host;
    rpc::RpcClient client(host);

    auto from_address = parser.getValue<std::string>("from_address");
    auto to_address = parser.getValue<std::string>("to_address");
    auto amount = parser.getValue<std::uint32_t>("amount");
    auto result = client.transaction(amount, from_address.c_str(), to_address.c_str());

    std::cout << "Remote call of transaction [" << result << "]" << std::endl;
}

void testConnection(const base::ProgramOptionsParser& parser)
{
    if(parser.hasOption("help")) {
        std::cout << parser.helpMessage() << std::endl;
        return;
    }
    auto host = parser.getValue<std::string>("host");
    LOG_INFO << "Try to connect to rpc server by: " << host;
    rpc::RpcClient client(host);

    // add rpc test message rpc method and add hand shake message
}

} // namespace

int main(int argc, char** argv)
{
    try {
        base::initLog(base::LogLevel::ALL, base::Sink::STDOUT);

        base::ProgramOptionsParser parser("rpc-client");
        parser.addFlag("version,v", "Print version of program and version of rpc protocol");

        auto get_balance_sub_parser =
            parser.createSubParser("get_balance", "use for get balance from remote by account address", getBalance);
        get_balance_sub_parser->addOption<std::string>("host,h", "address of remote host in format: \"<ip>:<port>\"");
        get_balance_sub_parser->addOption<std::string>("address,a", "account address");

        auto transaction_sub_parser =
            parser.createSubParser("transaction", "use for get balance from remote by account address", transaction);
        transaction_sub_parser->addOption<std::string>("host,h", "address of remote host in format: \"<ip>:<port>\"");
        transaction_sub_parser->addOption<std::string>("from_address,f", "from account address");
        transaction_sub_parser->addOption<std::string>("to_address,t", "to account address");
        transaction_sub_parser->addOption<std::uint32_t>("amount,m", "transaction money");

        auto test_sub_parser = parser.createSubParser("test_connection", "use test functions", testConnection);
        test_sub_parser->addOption<std::string>("host,h", "address of remote host in format: \"<ip>:<port>\"");

        parser.process(argc, argv);

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
        LOG_ERROR << "[exception caught in rpc-client] " << error.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        LOG_ERROR << "[unknown exception caught]";
        return base::config::EXIT_FAIL;
    }
    return base::config::EXIT_OK;
}
