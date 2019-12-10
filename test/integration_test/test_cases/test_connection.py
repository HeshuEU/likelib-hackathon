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
            "threads": 2
        },
        "nodes": [
        ]
    } 
    '''

    work_dir = os.path.join(_work_dir, "node_work_dir")
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)
    os.chdir(work_dir)
    _node_config_file = os.path.join(work_dir, "config.json")

    with open(_node_config_file, 'w') as node_config:
        node_config.write(node_config_file_content)

    try:
        return_code = subprocess.run([node_exec_path, "--config", _node_config_file], capture_output=True)
        return return_code.returncode
    except Exception:
        exit(1)


def client_run_fun(rpc_client_exec_path):
    work_dir = os.path.join(_work_dir, "client_work_dir")
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)
    os.chdir(work_dir)

    time_to_node_set_up = 5
    time.sleep(time_to_node_set_up)
    rpc_pipe = subprocess.run([rpc_client_exec_path, "test", "--host", "127.0.0.1:50051"], capture_output=True)

    print(rpc_pipe.stdout)
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
    node_process.kill()

    return client_process.exitcode
