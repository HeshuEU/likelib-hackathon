import argparse

import tester

import test_connection
import test_transfer
import test_contract

# import test_multi_network
# import test_multi_transfer

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Test execution params')
    parser.add_argument('-b', '--bin', type=str, help='path to folder with dependencies')
    parser.add_argument('-t', '--tests', type=str, default="",
                        help='pattern for test execution')

    args = parser.parse_args()

    return_code = tester.run_registered_test_cases(args.tests, args.bin)
    exit(return_code)
