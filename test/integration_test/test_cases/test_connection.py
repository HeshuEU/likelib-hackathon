import os
import shutil
import time
import subprocess
import multiprocessing as mp


_work_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test_connection")


def node_run_fun(node_exec_path):   
    node_config_file_content = '''
{
    "net": {
        "listen_addr": "0.0.0.0:20203",
        "public_port": 20203
    },
    "rpc": {
        "address": "0.0.0.0:50051"
    },
    "miner": {
        "threads": 4
    },
    "nodes": [
    ],
    "database": {
        "path": "likelib/database",
        "clean": false
    }
}
'''

    work_dir = os.path.join(_work_dir, "node_work_dir")
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)
    os.chdir(work_dir)
    _node_config_file = os.path.join(work_dir, "config.json")

    with open(_node_config_file, 'w') as node_config:
        node_config.write(node_config_file_content)

    test_timeout = 7
    try:
        return_code = subprocess.run([node_exec_path, "--config", _node_config_file], capture_output=True, timeout=test_timeout)
    except Exception:
        pass
    exit(0)


def client_run_fun(rpc_client_exec_path):
    work_dir = os.path.join(_work_dir, "client_work_dir")
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)
    os.chdir(work_dir)

    time_to_node_set_up = 5
    time.sleep(time_to_node_set_up)
    rpc_pipe = subprocess.run([rpc_client_exec_path, "test", "--host", "0.0.0.0:50051"], capture_output=True)

    if b"Test passed" in rpc_pipe.stdout and rpc_pipe.returncode == 0:
        exit(0)
    else:
        print(rpc_pipe.stderr)
        exit(1)


def main(node_exec_path, rpc_client_exec_path):
    if os.path.exists(_work_dir):
        shutil.rmtree(_work_dir, ignore_errors=True)
    os.makedirs(_work_dir)
    
    node_process = mp.Process(target=node_run_fun, args=[node_exec_path, ])
    client_process = mp.Process(target=client_run_fun, args=[rpc_client_exec_path, ])
    
    node_process.start()
    client_process.start()

    client_process.join()
    exit_code = client_process.exitcode
    client_process.close()
    
    node_process.join()
    node_process.close()

    return exit_code
