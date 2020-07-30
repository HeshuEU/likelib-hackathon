#include <boost/test/unit_test.hpp>

#include "base/hash.hpp"
#include "base/json.hpp"


BOOST_AUTO_TEST_CASE(inner_json_serialize)
{
    base::json::Value result;
    result["top_block_hash"] = base::json::Value::string("mH9qbu1tP3uG1Uvn66qL8Ad/S7uxd6Ap6FQ4SWJ4t/8=");
    result["top_block_number"] = base::json::Value::number(1);
    result["api_version"] = base::json::Value::number(2);
    auto test = result.serialize();
    std::cout << test;
}

BOOST_AUTO_TEST_CASE(inner_json_parse)
{
    const char* source_code = R"raw({
    "api_version" : 2,
    "top_block_hash" : "mH9qbu1tP3uG1Uvn66qL8Ad/S7uxd6Ap6FQ4SWJ4t/8=",
    "top_block_number" : 1
})raw";

    base::json::Value result = base::json::Value::parse(source_code);
    BOOST_CHECK_EQUAL(result["top_block_number"].as_number().to_uint64(), 1u);
}