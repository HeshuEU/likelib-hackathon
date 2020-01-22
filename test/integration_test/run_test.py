import os
import argparse
import glob

import tester

import test_connection
import test_transfer
import test_multi_network
import test_multi_transfer

# TODO: make script multi-platform

if __name__ == "__main__":

    node_exec_build_relative_path = "node"
    rpc_client_exec_build_relative_path = "rpc-client"


    parser = argparse.ArgumentParser(description='Test execution params')

    parser.add_argument('-r', '--rpc_client', type=str, default=rpc_client_exec_build_relative_path,
                        help='rpc-client execution file path')

    parser.add_argument('-n', '--node', type=str, default=node_exec_build_relative_path,
                        help='node execution file path')

    args = parser.parse_args()

    node_exec_path = args.node
    if not os.path.exists(node_exec_path):
        print(f"Not found file by path: {node_exec_path}")
        exit(2)

    rpc_client_exec_path = args.rpc_client
    if not os.path.exists(rpc_client_exec_path):
        print(f"Not found file by path: {rpc_client_exec_path}")
        exit(2)

    return_code = tester.run_registered_test_cases(node_exec_path, rpc_client_exec_path)
    exit(return_code)
