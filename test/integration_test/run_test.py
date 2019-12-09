import os
import datetime
import test_cases

work_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(work_dir)

# TODO: make crossplatform and movable path
os.environ["NODE_PATH"] = "/likelib/node"
os.environ["RPC_CLIENT_PATH"] = "/likelib/rpc-client"

if __name__=="__main__":
    return_code = 0
    for registered_case in test_cases.registered_test_cases:
        print(f"Test case [", registered_case["name"], "] started.")
        case_start_time = datetime.datetime.now()
        exit_code = registered_case["run"]()
        if exit_code == 0:
            print(f"Test case [", registered_case["name"], "] success.")
        else:
            print(f"Test case [", registered_case["name"], "] failed.")
            return_code = 1
        print(f"Test case [", registered_case["name"], "] execute time: ", datetime.datetime.now() - case_start_time)
    exit(return_code)

