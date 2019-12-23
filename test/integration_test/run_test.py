import os
import argparse
import glob

import tester

import test_connection
import test_transaction


# TODO: make script multi-platform

if __name__ == "__main__":
    candiadte_folders = glob.glob(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "*build*/"), recursive=False)

    if len(candiadte_folders) == 0:
        node_exec_build_relative_path = "node"
        rpc_client_exec_build_relative_path = "rpc-client"
    elif len(candiadte_folders) == 1:
        build_dir = candiadte_folders[0]
        node_exec_build_relative_path = os.path.join(
            build_dir, "src", "node", "node")
        rpc_client_exec_build_relative_path = os.path.join(
            build_dir, "src", "rpc-client", "rpc-client")
    else:
        print("Need to specify paths to test execute files. Run \"python3 run_test.py --help\" to read more.")
        exit(2)

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
