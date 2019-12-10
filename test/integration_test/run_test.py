import os
import argparse
import glob
import datetime

import test_cases as tc

# TODO: implement logger instedd print
# TODO: implement check functions with optional error messages
# TODO: make script multyplatform
WORK_DIR = os.path.dirname(os.path.abspath(__file__))


def run_registered_test_cases(node_exec_path, rpc_client_exec_path):
    os.chdir(WORK_DIR)

    success_tests = 0
    failed_tests = 0

    for registered_case in tc.registered_test_cases:
        registered_test_case_runner = registered_case["run"]
        registered_test_case_name = registered_case["name"]

        print(f"Test case [{registered_test_case_name}] started.")
        
        test_case_start_time = datetime.datetime.now()
        try:
            return_code = registered_test_case_runner(node_exec_path, rpc_client_exec_path)
        except Exception as e:
            print(f"Catch unexpected exception: {e}")
            return_code = 2
        test_case_execute_time = datetime.datetime.now() - test_case_start_time

        if return_code == 0:
            print(f"Test case [{registered_test_case_name}] success. Execute time: {test_case_execute_time}.")
            success_tests += 1 
        else:
            print(f"Test case [{registered_test_case_name}] failed. Execute time: {test_case_execute_time}.")
            failed_tests += 1

    all_tests = success_tests + failed_tests
    print(f"Started test cases: {all_tests}. Passed tests: {success_tests}. Failed tests: {failed_tests}")
    return failed_tests


if __name__=="__main__":
    candiadte_folders = glob.glob(os.path.join(WORK_DIR, "..", "..", "*build*/"), recursive=False)
    if len(candiadte_folders) == 0:
        node_exec_build_relative_path = "node"
        rpc_client_exec_build_relative_path = "rpc-client"
    elif len(candiadte_folders) == 1:
        build_dir=candiadte_folders[0]
        node_exec_build_relative_path = os.path.join(build_dir, "src", "node", "node")
        rpc_client_exec_build_relative_path = os.path.join(build_dir, "src", "rpc-client", "rpc-client")
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

    return_code = run_registered_test_cases(node_exec_path, rpc_client_exec_path)
    exit(return_code)
