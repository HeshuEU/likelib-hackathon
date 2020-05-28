import datetime
import re
import traceback

from .base import CheckFailedException
from .env import Env


# test case checks
def TEST_CHECK(boolean_value, *, message=""):
    if not boolean_value:
        traceback_list = traceback.format_stack()
        log_message = ""
        for i in range(len(traceback_list)):
            if (traceback_list[i].find('in test_case_runner') > 0) and (
                    traceback_list[i].find('return func(env)\n') > 0):
                for log_line in traceback_list[i + 1:-1]:
                    log_message = log_message + log_line
                break
        if message and len(message) > 0:
            raise CheckFailedException(
                f"Check failed: {message}.\n Trace:\n{log_message}")
        else:
            raise CheckFailedException(
                f"Check failed.\n Trace:\n{log_message}")


def TEST_CHECK_EQUAL(left, right, *, message=""):
    TEST_CHECK(left == right, message=message)


def TEST_CHECK_NOT_EQUAL(left, right, *, message=""):
    TEST_CHECK(left != right, message=message)


__enabled_tests = dict()
__disabled_tests = dict()


def test_case(test_name, disable=False):
    def test_case_register(func):
        def test_case_runner(dependencies_folder, run_folder):
            try:
                with Env(binary_path=dependencies_folder, run_folder=run_folder, test_name=test_name) as env:
                    return func(env)
            except Exception as error:
                print(error, flush=True)
                return 2

        if test_name in __enabled_tests.keys() or test_name in __disabled_tests.keys():
            raise Exception(f"Test with this name[{test_name}] is exists")

        if disable:
            print(f"Registered test case [{test_name}] is disabled", flush=True)
            __disabled_tests[test_name] = test_case_runner
        else:
            print(f"Registered test case [{test_name}] is enabled", flush=True)
            __enabled_tests[test_name] = test_case_runner

        return test_case_runner

    return test_case_register


def run_registered_test_cases(pattern, dependencies_folder, run_folder):
    success_tests = 0
    failed_tests = 0
    skipped_tests = 0

    try:
        matcher = re.compile(pattern)
    except Exception as e:
        matcher = None
        print(f"Invalid pattern: {e}", flush=True)
        exit(2)

    print(f"Enabled tests: {len(__enabled_tests)}. Disabled tests: {len(__disabled_tests)}.", flush=True)

    for registered_test_case_name in __enabled_tests:
        if matcher.match(registered_test_case_name) is None:
            skipped_tests += 1
            continue

        registered_test_case_runner = __enabled_tests[registered_test_case_name]

        print(f"Test case [{registered_test_case_name}] started.", flush=True)

        test_case_start_time = datetime.datetime.now()
        return_code = registered_test_case_runner(dependencies_folder, run_folder)
        test_case_execute_time = datetime.datetime.now() - test_case_start_time

        if return_code == 0:
            status = f"[{registered_test_case_name}] success"
            success_tests += 1
        else:
            status = f"[{registered_test_case_name}] failed"
            failed_tests += 1

        print(f"Test case {status}. Execute time: {test_case_execute_time}.", flush=True)

    all_tests = success_tests + failed_tests + skipped_tests
    print(
        f"All test cases: {all_tests}. Passed tests: {success_tests}. Failed tests: {failed_tests}. Skipped tests: {skipped_tests}.",
        flush=True)
    return failed_tests
