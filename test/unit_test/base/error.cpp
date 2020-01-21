#include <boost/test/unit_test.hpp>

#include "base/error.hpp"

#include <sstream>

BOOST_AUTO_TEST_CASE(error_constructor)
{
    std::string error_str = "Test err";
    base::Error te(error_str);
    BOOST_CHECK(te.toStdString() == error_str);
    BOOST_CHECK(std::string(te.what()) == error_str);
}


BOOST_AUTO_TEST_CASE(error_constructor_with_empty_string)
{
    std::string error_str = "";
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
    std::string error_str1 = "1247468$^)9#2DZSF;LM gdsmTest err/N/N \n";
    std::string error_str2 = "GFD #%*39F;LM gdsmTest err/N/N \n";
    std::string error_str3 = "1247468$^)9#2DZSF;LMdsfj#%()630mk";
    base::Error te1(error_str1);
    base::Error te2(error_str2);
    base::Error te3(error_str3);

    std::stringstream stream;
    std::string temp_str = "#)*fk%#0/n #%  sdf +_";
    stream << te1 << temp_str << te2 << te3;
    BOOST_CHECK(stream.str() == error_str1 + temp_str + error_str2 + error_str3);
}