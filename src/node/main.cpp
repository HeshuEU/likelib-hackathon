#include "node/hard_config.hpp"
#include "node/node.hpp"

#include "base/assert.hpp"
#include "base/config.hpp"
#include "base/log.hpp"
#include "base/program_options.hpp"

#include <boost/stacktrace.hpp>

#ifdef CONFIG_OS_FAMILY_UNIX
#include <cstring>
#endif

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <thread>

namespace
{

extern "C" void signalHandler(int signal)
{
    if (signal == SIGINT) {
        LOG_INFO << "SIGINT caught. Exit.";
        base::flushLog();
        std::_Exit(base::config::EXIT_OK); // TODO: a clean exit
    }
    else {
        LOG_INFO << "Signal caught: " << signal
#ifdef CONFIG_OS_FAMILY_UNIX
                 << " (" << strsignal(signal) << ")"
#endif
                 << '\n'
                 << boost::stacktrace::stacktrace();
        base::flushLog();
        std::_Exit(base::config::EXIT_FAIL);
    }
}


void atExitHandler()
{
    LOG_INFO << "Node shutdown";
    base::flushLog();
}


base::json::Value load_config(std::filesystem::path config_path)
{
    std::ifstream input;
    input.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        input.open(config_path);
    }
    catch (const std::exception& e) {
        RAISE_ERROR(base::InaccessibleFile, e.what());
    }
    return base::json::Value::parse(input);
}

} // namespace

int main(int argc, char** argv)
{
    try {
        base::initLog(base::Sink::FILE | base::Sink::STDOUT);
        LOG_INFO << "Node startup";

        // set up options parser
        base::ProgramOptionsParser parser;
        parser.addOption<std::string>("config,c", config::CONFIG_PATH, "Path to config file");

        // process options
        parser.process(argc, argv);
        if (parser.hasOption("help")) {
            std::cout << parser.helpMessage() << std::endl;
            return base::config::EXIT_OK;
        }

        auto config_file_path = parser.getValue<std::string>("config");
        if (!std::filesystem::exists(config_file_path)) {
            LOG_ERROR << "Config file does not exist by path \"" << config_file_path << '"';
            return base::config::EXIT_FAIL;
        }
        else {
            LOG_INFO << "Found config file by path \"" << config_file_path << '"';
        }

        // handlers initialization
        // setup handler for all signal types defined in Standard, expect SIGABRT. Not all POSIX signals
        for (auto signal_code : { SIGTERM, SIGSEGV, SIGINT, SIGILL, SIGFPE }) {
            [[maybe_unused]] auto result = std::signal(signal_code, signalHandler);
            ASSERT_SOFT(result != SIG_ERR);
        }

        {
            [[maybe_unused]] auto result = std::atexit(atExitHandler);
            ASSERT_SOFT(result == 0);
        }

        //=====================
        auto exe_config = load_config(config_file_path);
        Node node(exe_config);
        node.run();
        //=====================
        while (true) {
            constexpr auto timeout = std::chrono::hours(24);
            std::this_thread::sleep_for(timeout);
        }
        return base::config::EXIT_OK;
    }
    catch (const std::exception& error) {
        LOG_ERROR << "[exception caught in main] " << error.what();
        return base::config::EXIT_FAIL;
    }
    catch (...) {
        LOG_ERROR << "[unknown exception caught]";
        return base::config::EXIT_FAIL;
    }
}
