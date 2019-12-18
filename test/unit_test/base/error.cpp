#include <boost/test/unit_test.hpp>

#include "base/error.hpp"

#include <sstream>

BOOST_AUTO_TEST_CASE(error_usage1)
{
    base::Error te("Test err");
    BOOST_CHECK(te.toStdString() == "Test err");
    BOOST_CHECK(std::string(te.what()) == "Test err");
}


BOOST_AUTO_TEST_CASE(error_constructor_copy)
{
    base::Error te("Test err copy");
    base::Error te2(te);
    BOOST_CHECK(te.toStdString() == "Test err copy" && te.toStdString() == te2.toStdString());
    BOOST_CHECK(std::string(te.what()) == "Test err copy" && std::string(te.what()) == std::string(te2.what()));
}


BOOST_AUTO_TEST_CASE(error_constructor_move)
{
    base::Error te("Test err move");
    base::Error te2(std::move(te));
    BOOST_CHECK(te2.toStdString() == "Test err move" && te.toStdString() == "");
    BOOST_CHECK(std::string(te2.what()) == "Test err move" && std::string(te.what()) == "");
}


BOOST_AUTO_TEST_CASE(error_operator_equal)
{
    base::Error te("Test err");
    base::Error te2("Tes1 err");
    te2 = te;
    BOOST_CHECK(te.toStdString() == "Test err" && te.toStdString() == te2.toStdString());
    BOOST_CHECK(std::string(te.what()) == "Test err" && std::string(te.what()) == std::string(te2.what()));
}


BOOST_AUTO_TEST_CASE(error_operator_equal_move)
{
    base::Error te("Test err");
    base::Error te2("Tes1 err");
    te2 = std::move(te);
    BOOST_CHECK(te2.toStdString() == "Test err" && te.toStdString() == "");
    BOOST_CHECK(std::string(te2.what()) == "Test err" && std::string(te.what()) == "");
}


BOOST_AUTO_TEST_CASE(error_operator_output)
{
    base::Error te("1247468$^)9#2DZSF;LM gdsmTest err/N/N \n");
    std::stringstream stream;
    stream << te;
    std::string what;
    what = stream.str();
    BOOST_CHECK(what == "1247468$^)9#2DZSF;LM gdsmTest err/N/N \n");
}