#include "soft_config.hpp"
#include "hard_config.hpp"

#include "base/config.hpp"
#include "base/log.hpp"
#include "base/assert.hpp"
#include "net/network.hpp"
#include "net/endpoint.hpp"

#ifdef CONFIG_OS_FAMILY_UNIX
#include <cstring>
#endif

#include <csignal>
#include <cstdlib>
#include <exception>
#include <thread>


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

        // handlers initialization

        // setup handler for all signal types defined in Standard. Not all POSIX signals
        for(auto signal_code: {SIGTERM, SIGSEGV, SIGINT, SIGILL, SIGFPE}) {
            ASSERT_SOFT(std::signal(signal_code, signalHandler) != SIG_ERR);
        }

        ASSERT_SOFT(std::atexit(atExitHandler) == 0);

        //=====================

        SoftConfig exe_config(config::CONFIG_PATH);

        net::Network manager(net::Endpoint{exe_config.get<std::string>("listen_address")});
        manager.run();

        std::vector<net::Endpoint> nodes;
        for(const auto& node_ip_string: exe_config.getVector<std::string>("nodes")) {
            nodes.emplace_back(node_ip_string);
        }
        manager.connect(nodes);

        std::this_thread::sleep_for(std::chrono::seconds(45));

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