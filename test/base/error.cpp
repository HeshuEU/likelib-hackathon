#include <boost/test/unit_test.hpp>

#include "base/error.hpp"

#include <sstream>

BOOST_AUTO_TEST_CASE(error_usage1)
{
    std::string error_str = "Test err";
    base::Error te(error_str);
    BOOST_CHECK(te.toStdString() == error_str);
    BOOST_CHECK(std::string(te.what()) == error_str);
}


BOOST_AUTO_TEST_CASE(error_constructor_copy)
{
    std::string error_str = "Test err copy";
    base::Error te(error_str);
    base::Error te2(te);
    BOOST_CHECK(te.toStdString() == error_str && te.toStdString() == te2.toStdString());
    BOOST_CHECK(std::string(te.what()) == error_str && std::string(te.what()) == std::string(te2.what()));
}


BOOST_AUTO_TEST_CASE(error_constructor_move)
{
    std::string error_str = "Test err move";
    base::Error te(error_str);
    base::Error te2(std::move(te));
    BOOST_CHECK(te2.toStdString() == error_str && te.toStdString() == "");
    BOOST_CHECK(std::string(te2.what()) == error_str && std::string(te.what()) == "");
}


BOOST_AUTO_TEST_CASE(error_operator_equal)
{
    std::string error_str = "Test err";
    base::Error te(error_str);
    base::Error te2("Tes1 err");
    te2 = te;
    BOOST_CHECK(te.toStdString() == error_str && te.toStdString() == te2.toStdString());
    BOOST_CHECK(std::string(te.what()) == error_str && std::string(te.what()) == std::string(te2.what()));
}


BOOST_AUTO_TEST_CASE(error_operator_equal_move)
{
    std::string error_str = "Test err";
    base::Error te(error_str);
    base::Error te2("Tes1 err");
    te2 = std::move(te);
    BOOST_CHECK(te2.toStdString() == error_str && te.toStdString() == "");
    BOOST_CHECK(std::string(te2.what()) == error_str && std::string(te.what()) == "");
}


BOOST_AUTO_TEST_CASE(error_operator_output)
{
    std::string error_str = "1247468$^)9#2DZSF;LM gdsmTest err/N/N \n";
    base::Error te(error_str);
    std::stringstream stream;
    stream << te;
    std::string what;
    what = stream.str();
    BOOST_CHECK(what == error_str);
}