#include "client.hpp"

#include "config.hpp"

#include "base/error.hpp"
#include "base/program_options.hpp"


int main(int argc, const char** argv)
{
    base::initLog(base::Sink::FILE);
    base::ProgramOptionsParser options_parser;
    options_parser.addFlag("version,v", "Print version of program");
    options_parser.process(argc, argv);

    if (options_parser.hasOption("help")) {
        std::cout << options_parser.helpMessage() << std::endl;
        return base::config::EXIT_OK;
    }

    if (options_parser.hasOption("version")) {
        std::cout << "Likelib client v" << config::CLIENT_VERSION << std::endl;
        return base::config::EXIT_OK;
    }

    Client client;
    client.run();

    return base::config::EXIT_OK;
}