#include <boost/test/unit_test.hpp>

#include "base/property_tree.hpp"

#include <filesystem>
#include <fstream>
#include <string>


BOOST_AUTO_TEST_CASE(property_tree)
{
    {
        std::ofstream output("config.json");
        output << R"({"name": "test_name"})";
    }

    base::PropertyTree config = base::readConfig("config.json");
    BOOST_CHECK_EQUAL(config.get<std::string>("name"), "test_name");
    std::filesystem::remove("config.json");
}