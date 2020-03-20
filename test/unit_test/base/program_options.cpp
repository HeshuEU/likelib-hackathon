#include "boost/test/unit_test.hpp"

#include "base/program_options.hpp"

BOOST_AUTO_TEST_CASE(program_options_empty_test_1)
{
    int argc = 1;
    char test1[] = "unit_test.exe";
    char* argv[] = { test1 };

    base::ProgramOptionsParser parser;
    parser.addFlag("demonize,d", "demonize application start");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(!parser.hasOption("demonize"));
}

BOOST_AUTO_TEST_CASE(program_options_flag_test)
{
    int argc = 2;
    char test1[] = "unit_test.exe";
    char test2[] = "-d";
    char* argv[] = { test1, test2 };

    base::ProgramOptionsParser parser;
    parser.addFlag("demonize,d", "demonize application start");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("demonize"));
}

BOOST_AUTO_TEST_CASE(program_options_uint_test)
{
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "-p";
    char test3[] = "900";
    char* argv[] = { test1, test2, test3 };

    base::ProgramOptionsParser parser;
    parser.addOption<std::uint32_t>("processors,p", "Processors count");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("processors"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(900, parser.getValue<std::uint32_t>("processors")));
}

BOOST_AUTO_TEST_CASE(program_options_default_uint_test)
{
    uint32_t test_target = 900;

    int argc = 1;
    char test1[] = "unit_test.exe";
    char* argv[] = { test1 };

    base::ProgramOptionsParser parser;
    parser.addOption<std::uint32_t>("processors,p", test_target, "Processors count");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("processors"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(900, parser.getValue<std::uint32_t>("processors")));
}

BOOST_AUTO_TEST_CASE(program_options_required_uint_test)
{
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "-p";
    char test3[] = "900";
    char* argv[] = { test1, test2, test3 };

    base::ProgramOptionsParser parser;
    parser.addRequiredOption<std::uint32_t>("processors,p", "Processors count");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("processors"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(900, parser.getValue<std::uint32_t>("processors")));
}

BOOST_AUTO_TEST_CASE(program_options_int_test)
{
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "--number";
    char test3[] = "-809900";
    char* argv[] = { test1, test2, test3 };

    base::ProgramOptionsParser parser;
    parser.addOption<std::int32_t>("number,n", "Input number");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("number"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(-809900, parser.getValue<std::int32_t>("number")));
}

BOOST_AUTO_TEST_CASE(program_options_default_int_test)
{
    int32_t test_target = -809900;

    int argc = 1;
    char test1[] = "unit_test.exe";
    char* argv[] = { test1 };

    base::ProgramOptionsParser parser;
    parser.addOption<std::int32_t>("number,n", test_target, "Input number");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("number"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(test_target, parser.getValue<std::int32_t>("number")));
}

BOOST_AUTO_TEST_CASE(program_options_required_int_test)
{
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "--number";
    char test3[] = "-809900";
    char* argv[] = { test1, test2, test3 };

    base::ProgramOptionsParser parser;
    parser.addRequiredOption<std::int32_t>("number,n", "Input number");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("number"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(-809900, parser.getValue<std::int32_t>("number")));
}

BOOST_AUTO_TEST_CASE(program_options_double_test)
{
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "--money";
    char test3[] = "1.1";
    char* argv[] = { test1, test2, test3 };

    base::ProgramOptionsParser parser;
    parser.addOption<double>("money,m", "Money count");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("money"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(1.1, parser.getValue<double>("money")));
}

BOOST_AUTO_TEST_CASE(program_options_default_double_test)
{
    double test_target = 1.1;

    int argc = 1;
    char test1[] = "unit_test.exe";
    char* argv[] = { test1 };

    base::ProgramOptionsParser parser;
    parser.addOption<double>("money,m", test_target, "Money count");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("money"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(test_target, parser.getValue<double>("money")));
}

BOOST_AUTO_TEST_CASE(program_options_required_double_test)
{
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "-m";
    char test3[] = "1.1";
    char* argv[] = { test1, test2, test3 };

    base::ProgramOptionsParser parser;
    parser.addRequiredOption<double>("money,m", "Money count");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("money"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(1.1, parser.getValue<double>("money")));
}

BOOST_AUTO_TEST_CASE(program_options_string_test)
{
    std::string target = "GJSHDGI32mvdsjb12BFA";
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "-h";
    char* argv[] = { test1, test2, target.data() };

    base::ProgramOptionsParser parser;
    parser.addOption<std::string>("hash,h", "Hash string");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("hash"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(target, parser.getValue<std::string>("hash")));
}

BOOST_AUTO_TEST_CASE(program_options_default_string_test)
{
    std::string target = "GJSHDGI32mvdsjb12BFA";
    int argc = 1;
    char test1[] = "unit_test.exe";
    char* argv[] = { test1 };

    base::ProgramOptionsParser parser;
    parser.addOption<std::string>("hash,h", target, "Hash string");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("hash"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(target, parser.getValue<std::string>("hash")));
}

BOOST_AUTO_TEST_CASE(program_options_required_string_test)
{
    std::string target = "GJSHDGI32mvdsjb12BFA";
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "--hash";
    char* argv[] = { test1, test2, target.data() };

    base::ProgramOptionsParser parser;
    parser.addRequiredOption<std::string>("hash,h", "Hash string");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("hash"));

    BOOST_CHECK_NO_THROW(BOOST_CHECK_EQUAL(target, parser.getValue<std::string>("hash")));
}

BOOST_AUTO_TEST_CASE(program_options_help_message)
{
    std::string test_target_1 = "Hash string";
    std::string test_target_2 = "Money count";
    std::string test_target_3 = "Input number";
    std::int32_t test_target = 3;

    int argc = 2;
    char test1[] = "unit_test.exe";
    char test2[] = "--help";
    char* argv[] = { test1, test2 };

    base::ProgramOptionsParser parser;
    parser.addOption<std::string>("hash,h", test_target_1);
    parser.addOption<double>("money,m", test_target_2);
    parser.addOption<std::int32_t>("number,n", test_target, test_target_3);
    auto temp = parser.helpMessage();
    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("help"));
    auto help_message = parser.helpMessage();

    BOOST_CHECK_NE(std::string::npos, help_message.find(test_target_1));
    BOOST_CHECK_NE(std::string::npos, help_message.find(test_target_2));
    BOOST_CHECK_NE(std::string::npos, help_message.find(test_target_3));
}

BOOST_AUTO_TEST_CASE(program_options_help_message_bad_declaration)
{
    std::string test_target_1 = "Hash string";
    std::string test_target_2 = "Money count";
    std::string test_target_3 = "Input number";
    std::int32_t test_target = 3;

    int argc = 2;
    char test1[] = "unit_test.exe";
    char test2[] = "--help";
    char* argv[] = { test1, test2 };

    base::ProgramOptionsParser parser;
    parser.addRequiredOption<std::string>("hash,h", test_target_1);
    parser.addRequiredOption<double>("money.m", test_target_2);
    parser.addOption<std::int32_t>("number,n", test_target, test_target_3);

    BOOST_CHECK_THROW(parser.process(argc, argv), base::ParsingError);
}

BOOST_AUTO_TEST_CASE(program_options_required_throw)
{
    int argc = 1;
    char test1[] = "unit_test.exe";
    char* argv[] = { test1 };

    base::ProgramOptionsParser parser;
    parser.addRequiredOption<std::string>("hash", "Hash string");

    BOOST_CHECK_THROW(parser.process(argc, argv), base::ParsingError);
}

BOOST_AUTO_TEST_CASE(program_options_incorrect_input_params)
{
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "--temp";
    char test3[] = "-809900";
    char* argv[] = { test1, test2, test3 };

    base::ProgramOptionsParser parser;
    parser.addOption<std::int32_t>("number,n", "Input number");

    BOOST_CHECK_THROW(parser.process(argc, argv), base::ParsingError);
}

BOOST_AUTO_TEST_CASE(program_options_incorrect_type_exception)
{
    int argc = 3;
    char test1[] = "unit_test.exe";
    char test2[] = "--number";
    char test3[] = "-809900";
    char* argv[] = { test1, test2, test3 };

    base::ProgramOptionsParser parser;
    parser.addOption<std::int32_t>("number,n", "Input number");

    BOOST_CHECK_NO_THROW(parser.process(argc, argv));

    BOOST_CHECK(parser.hasOption("number"));
    BOOST_CHECK_THROW(BOOST_CHECK_EQUAL(-809900, parser.getValue<std::uint32_t>("number")), base::InvalidArgument);
}

int walletProcessForTestSubParser1(const base::ProgramOptionsParser& parser)
{
    if (parser.hasOption("help")) {
        BOOST_CHECK(true);
    }
    else {
        BOOST_CHECK(false);
    }
    return 0;
}
