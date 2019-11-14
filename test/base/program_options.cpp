#include "boost/test/unit_test.hpp"

#include "base/program_options.hpp"

BOOST_AUTO_TEST_CASE(program_options_flag_test)
{
    int argc = 2;
    char test1[] = {"test.exe"};
    char test2[] = {"-d"};
    char* argv[] = {test1, test2};

    base::ProgramOptionsParser parser;
    parser.addFlagOption("demonize,d", "demonize application start");

    parser.process(argc, argv);

    BOOST_CHECK(parser.hasOption("demonize"));
}

BOOST_AUTO_TEST_CASE(program_options_uint_test)
{
    int argc = 3;
    char test1[] = {"test.exe"};
    char test2[] = {"-p"};
    char test3[] = {"900"};
    char* argv[] = {test1, test2, test3};

    base::ProgramOptionsParser parser;
    parser.addUintOption("processors,p", "Processors count");

    parser.process(argc, argv);

    BOOST_CHECK(parser.hasOption("processors"));

    BOOST_CHECK_EQUAL(900, parser.getUint("processors"));
}

BOOST_AUTO_TEST_CASE(program_options_int_test)
{
    int argc = 3;
    char test1[] = {"test.exe"};
    char test2[] = {"--number"};
    char test3[] = {"-809900"};
    char* argv[] = {test1, test2, test3};

    base::ProgramOptionsParser parser;
    parser.addIntOption("number,n", "Input number");

    parser.process(argc, argv);

    BOOST_CHECK(parser.hasOption("number"));

    BOOST_CHECK_EQUAL(-809900, parser.getInt("number"));
}

BOOST_AUTO_TEST_CASE(program_options_string_test)
{
    std::string target = "GJSHDGI32mvdsjb12BFA";
    int argc = 3;
    char test1[] = {"test.exe"};
    char test2[] = {"--hash"};
    char* argv[] = {test1, test2, target.data()};

    base::ProgramOptionsParser parser;
    parser.addStringOption("hash", "Hash string");

    parser.process(argc, argv);

    BOOST_CHECK(parser.hasOption("hash"));

    BOOST_CHECK_EQUAL(target, parser.getString("hash"));
}

BOOST_AUTO_TEST_CASE(program_options_incorrect_input_params)
{
    int argc = 3;
    char test1[] = {"test.exe"};
    char test2[] = {"--temp"};
    char test3[] = {"-809900"};
    char* argv[] = {test1, test2, test3};

    base::ProgramOptionsParser parser;
    parser.addIntOption("number,n", "Input number");

    BOOST_CHECK_THROW(parser.process(argc, argv), base::ParsingError);
}

BOOST_AUTO_TEST_CASE(program_options_incorrect_type_exception)
{
    int argc = 3;
    char test1[] = {"test.exe"};
    char test2[] = {"--number"};
    char test3[] = {"-809900"};
    char* argv[] = {test1, test2, test3};

    base::ProgramOptionsParser parser;
    parser.addIntOption("number,n", "Input number");

    parser.process(argc, argv);

    BOOST_CHECK(parser.hasOption("number"));
    BOOST_CHECK_THROW(BOOST_CHECK_EQUAL(-809900, parser.getUint("number")), base::InvalidArgument);
}
