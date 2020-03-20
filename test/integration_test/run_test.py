import os
import argparse

import tester

import test_connection
import test_transfer
# import test_multi_network
# import test_multi_transfer


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Test execution params')
    parser.add_argument('-c', '--client', type=str, default="client",
                        help='client execution file path')
    parser.add_argument('-n', '--node', type=str, default="node",
                        help='node execution file path')
    parser.add_argument('-t', '--tests', type=str, default="",
                        help='pattern for test execution')
    parser.add_argument('-e', '--evm', type=str, default="libevmone.so.0.4",
                        help='evm execution file path')

    args = parser.parse_args()

    if not os.path.exists(args.node):
        print(f"Not found node executalble file by path: {args.node}")
        exit(2)

    if not os.path.exists(args.client):
        print(
            f"Not found rpc client executalble file by path: {args.client}")
        exit(2)

    if not os.path.exists(args.evm):
        print(
            f"Not found evm shared library file by path: {args.evm}")
        exit(2)

    return_code = tester.run_registered_test_cases(
        args.tests, os.path.abspath(args.node), os.path.abspath(args.client), os.path.abspath(args.evm))
    exit(return_code)
