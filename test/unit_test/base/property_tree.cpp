#include <boost/test/unit_test.hpp>

#include "base/property_tree.hpp"

#include <filesystem>
#include <fstream>
#include <string>


BOOST_AUTO_TEST_CASE(property_tree_usage1)
{
    {
        std::ofstream output("config.json");
        output << R"({
    "name": "test_name",
    "array": [1, 2, 4, 8, 16, 256]
})";
    }

    base::PropertyTree config = base::readConfig("config.json");

    BOOST_CHECK(config.hasKey("name"));
    BOOST_CHECK(config.hasKey("array"));

    BOOST_CHECK_EQUAL(config.get<std::string>("name"), "test_name");

    const std::vector<int> right_values{ 1, 2, 4, 8, 16, 256 };
    BOOST_CHECK(config.getVector<int>("array") == right_values);

    std::filesystem::remove("config.json");
}


BOOST_AUTO_TEST_CASE(property_tree_usage2)
{
    {
        std::ofstream output("config.json");
        output << R"({
    "name": " D1ff1C u^#4lt_&43n2Ame /",
    "array": [1.1, 2.5, 4.7274, 8.57840, 16, 256.0]
})";
    }

    base::PropertyTree config = base::readConfig("config.json");

    BOOST_CHECK(config.hasKey("name"));
    BOOST_CHECK(config.hasKey("array"));

    BOOST_CHECK_EQUAL(config.get<std::string>("name"), " D1ff1C u^#4lt_&43n2Ame /");

    const std::vector<double> right_values{ 1.1, 2.5, 4.7274, 8.57840, 16, 256.0 };
    BOOST_CHECK(config.getVector<double>("array") == right_values);

    std::filesystem::remove("config.json");
}


BOOST_AUTO_TEST_CASE(property_tree_usage3)
{
    {
        std::ofstream output("config.json");
        output << R"({
    "number1": 468968574,
    "number2": 468968574.765678,
    "number3": false,
    "array": [true, false, true, true, true, false]
})";
    }

    base::PropertyTree config = base::readConfig("config.json");

    BOOST_CHECK(config.hasKey("number1"));
    BOOST_CHECK(config.hasKey("number2"));
    BOOST_CHECK(config.hasKey("number3"));
    BOOST_CHECK(config.hasKey("array"));

    BOOST_CHECK_EQUAL(config.get<long long>("number1"), 468968574);
    BOOST_CHECK_EQUAL(config.get<double>("number2"), 468968574.765678);
    BOOST_CHECK_EQUAL(config.get<bool>("number3"), false);

    const std::vector<bool> right_values{ true, false, true, true, true, false };
    BOOST_CHECK(config.getVector<bool>("array") == right_values);

    std::filesystem::remove("config.json");
}


BOOST_AUTO_TEST_CASE(property_tree_set_value)
{
    base::PropertyTree root;
    root.add("lololo",  5);

    base::PropertyTree sub;
    sub.add("temp1", "hello");
    sub.add("temp2",  9);

    root.add("lol", sub);

    std::cout << root.toString();
}