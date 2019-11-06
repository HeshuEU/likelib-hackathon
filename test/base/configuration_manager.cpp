#include <boost/test/unit_test.hpp>

#include "base/configuration_manager.hpp"

#include <filesystem>
#include <fstream>
#include <string>


BOOST_AUTO_TEST_CASE(configuration_manager)
{
    {
        std::ofstream output("config.json");
        output << R"({"name": "test_name"})";
    }

    base::Configuration config = base::readConfig("config.json");
    BOOST_CHECK_EQUAL(config.get<std::string>("name"), "test_name");
    std::filesystem::remove("config.json");
}