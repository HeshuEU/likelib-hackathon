#include "soft_config.hpp"
#include "hard_config.hpp"

#include "base/program_options.hpp"
#include "base/config.hpp"
#include "base/log.hpp"
#include "base/assert.hpp"
#include "bc/blockchain.hpp"
#include "bc/general_server_service.hpp"
#include "net/network.hpp"
#include "net/endpoint.hpp"
#include "rpc/rpc.hpp"

#ifdef CONFIG_OS_FAMILY_UNIX
#include <cstring>
#endif

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <thread>
#include <filesystem>


namespace
{

extern "C" void signalHandler(int signal)
{
    LOG_INFO << "Signal caught: " << signal
#ifdef CONFIG_OS_FAMILY_UNIX
             << " (" << strsignal(signal) << ")"
#endif
#ifdef CONFIG_IS_DEBUG
             << '\n'
             << boost::stacktrace::stacktrace()
#endif
        ;
    std::abort();
}


void atExitHandler()
{
    LOG_INFO << "Node shutdown";
}

} // namespace

int main(int argc, char** argv)
{
    try {
        base::initLog(base::LogLevel::ALL, base::Sink::STDOUT | base::Sink::FILE);
        LOG_INFO << "Node startup";

        // set up options parser
        base::ProgramOptionsParser parser;
        parser.addOption<std::string>("config,c", config::CONFIG_PATH, "Path to config file");

        // process options
        parser.process(argc, argv);
        if(parser.hasOption("help")) {
            std::cout << parser.helpMessage() << std::endl;
            return base::config::EXIT_OK;
        }

        auto config_file_path = parser.getValue<std::string>("config");

        if(!std::filesystem::exists(config_file_path)) {
            LOG_ERROR << "[config file is not exists] input file path: " << parser.getValue<std::string>("config");
            return base::config::EXIT_FAIL;
        }
        else {
            LOG_INFO << "Found config file by path: " << config_file_path;
        }

        // handlers initialization

        // setup handler for all signal types defined in Standard. Not all POSIX signals
        for(auto signal_code: {SIGTERM, SIGSEGV, SIGINT, SIGILL, SIGFPE}) {
            ASSERT_SOFT(std::signal(signal_code, signalHandler) != SIG_ERR);
        }

        ASSERT_SOFT(std::atexit(atExitHandler) == 0);

        //=====================

        SoftConfig exe_config(config_file_path);

        bc::Blockchain blockchain;

        net::Network network(net::Endpoint{exe_config.get<std::string>("listen_address")},
            exe_config.get<unsigned short>("public_server_port"));

        blockchain.setNetwork(&network);
        network.setBlockchain(&blockchain);

        network.run();

        std::vector<net::Endpoint> nodes;
        for(const auto& node_ip_string: exe_config.getVector<std::string>("nodes")) {
            nodes.emplace_back(node_ip_string);
        }
        network.connect(nodes);

        rpc::RpcServer server(exe_config.get<std::string>("rpc_address"));
        server.run();
        LOG_INFO << "RPC server started: " << exe_config.get<std::string>("rpc_address");

        std::this_thread::sleep_for(std::chrono::seconds(4500));

        return base::config::EXIT_OK;
    }
    catch(const std::exception& error) {
        LOG_ERROR << "[exception caught in main] " << error.what();
        return base::config::EXIT_FAIL;
    }
    catch(...) {
        LOG_ERROR << "[unknown exception caught]";
        return base::config::EXIT_FAIL;
    }
}
