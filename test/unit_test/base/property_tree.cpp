#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <fstream>
#include <string>

BOOST_AUTO_TEST_CASE(property_tree_read_json_str)
{
    const auto json_str = R"({
    "test_string": "hello world!",
    "test_double": 468968574.765678,
    "test_uint": 4689685,
    "test_int": -10,
    "test_bool": false,
    "test_object": {
        "test_string2": "hello world!hello world!",
        "test_uint2": 777
    },
    "test_boolean_array": [true, false, true, true, true, false],
    "test_object_array": [
        {
            "name": "test1"
        },
        {
            "name": "test2"
        },
        {
            "name": "test3"
        }
    ]
})";

    auto root = base::parseJson(json_str);

    BOOST_CHECK(root.hasKey("test_string"));
    BOOST_CHECK(root.isString("test_string"));
    BOOST_CHECK_EQUAL(root.getString("test_string"), std::string("hello world!"));

    BOOST_CHECK(root.hasKey("test_double"));
    BOOST_CHECK(root.isDouble("test_double"));

    BOOST_CHECK(root.hasKey("test_uint"));
    BOOST_CHECK(root.isUint("test_uint"));
    BOOST_CHECK_EQUAL(root.getUint("test_uint"), 4689685u);

    BOOST_CHECK(root.hasKey("test_int"));
    BOOST_CHECK(root.isInt("test_int"));
    BOOST_CHECK_EQUAL(root.getInt("test_int"), -10);

    BOOST_CHECK(root.hasKey("test_bool"));
    BOOST_CHECK(root.isBool("test_bool"));
    BOOST_CHECK_EQUAL(root.getBool("test_bool"), false);

    BOOST_CHECK(root.hasKey("test_object"));
    BOOST_CHECK(root.isObject("test_object"));
    auto sub = root.getSubTree("test_object");
    BOOST_CHECK(sub.hasKey("test_string2"));
    BOOST_CHECK(sub.isString("test_string2"));
    BOOST_CHECK_EQUAL(sub.getString("test_string2"), std::string("hello world!hello world!"));

    BOOST_CHECK(sub.hasKey("test_uint2"));
    BOOST_CHECK(sub.isUint("test_uint2"));
    BOOST_CHECK_EQUAL(sub.getUint("test_uint2"), 777u);

    const std::vector<bool> target_bools = { true, false, true, true, true, false };

    BOOST_CHECK(root.hasKey("test_boolean_array"));
    BOOST_CHECK(root.isArray("test_boolean_array"));
    //    for(const auto& a : root.getSubTree("test_boolean_array")){
    //        a.as
    //    }

    BOOST_CHECK(root.hasKey("test_object_array"));
    BOOST_CHECK(root.isArray("test_object_array"));
}


BOOST_AUTO_TEST_CASE(property_tree_create_json_str)
{
    base::PropertyTree sub;
    sub.addMember("test_string", base::PropertyTree::string("hello world!"));
    sub.addMember("test_uint", base::PropertyTree::number(10U));
    sub.addMember("test_int", base::PropertyTree::number(-2020));

    base::PropertyTree root;
    root.addMember("test_object", sub);

}

//
// BOOST_AUTO_TEST_CASE(property_tree_usage2)
//{
//    {
//        std::ofstream output("config.json");
//        output << R"({
//    "name": " D1ff1C u^#4lt_&43n2Ame /",
//    "array": [1.1, 2.5, 4.7274, 8.57840, 16, 256.0]
//})";
//    }
//
//    base::PropertyTree config = base::readConfig("config.json");
//
//    BOOST_CHECK(config.hasKey("name"));
//    BOOST_CHECK(config.hasKey("array"));
//
//    BOOST_CHECK_EQUAL(config.get<std::string>("name"), " D1ff1C u^#4lt_&43n2Ame /");
//
//    const std::vector<double> right_values{ 1.1, 2.5, 4.7274, 8.57840, 16, 256.0 };
//    BOOST_CHECK(config.getVector<double>("array") == right_values);
//
//    std::filesystem::remove("config.json");
//}
//
//
// BOOST_AUTO_TEST_CASE(property_tree_usage3)
//{
//    {
//        std::ofstream output("config.json");
//        output << R"({
//    "number1": 468968574,
//    "number2": 468968574.765678,
//    "number3": false,
//    "array": [true, false, true, true, true, false]
//})";
//    }
//
//    base::PropertyTree config = base::readConfig("config.json");
//
//    BOOST_CHECK(config.hasKey("number1"));
//    BOOST_CHECK(config.hasKey("number2"));
//    BOOST_CHECK(config.hasKey("number3"));
//    BOOST_CHECK(config.hasKey("array"));
//
//    BOOST_CHECK_EQUAL(config.get<long long>("number1"), 468968574);
//    BOOST_CHECK_EQUAL(config.get<double>("number2"), 468968574.765678);
//    BOOST_CHECK_EQUAL(config.get<bool>("number3"), false);
//
//    const std::vector<bool> right_values{ true, false, true, true, true, false };
//    BOOST_CHECK(config.getVector<bool>("array") == right_values);
//
//    std::filesystem::remove("config.json");
//}
//
//
// BOOST_AUTO_TEST_CASE(property_tree_set_value)
//{
//    base::PropertyTree root;
//    root.add("lololo",  5);
//
//    base::PropertyTree sub;
//    sub.add("temp1", "hello");
//    sub.add("temp2",  9);
//
//    root.add("lol", sub);
//
//    std::cout << root.toString();
//}
