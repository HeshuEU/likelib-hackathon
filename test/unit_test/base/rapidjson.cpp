#include <boost/test/unit_test.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


BOOST_AUTO_TEST_CASE(rapid_serialize)
{
    std::string test_1;
    rapidjson::Document response(rapidjson::kObjectType);
    {
        const auto test = "bspdhabfsbdfv";
        test_1 = std::string(test);
    }

    response.AddMember("test", rapidjson::StringRef(test_1.c_str()), response.GetAllocator());
    std::string output;
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        response.Accept(writer);
        output = buffer.GetString();
    }
}