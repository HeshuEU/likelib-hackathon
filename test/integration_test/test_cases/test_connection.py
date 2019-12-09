import os
import time
import subprocess
import multiprocessing as mp


_work_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test_case_1")
os.makedirs(_work_dir)


def node_run_fun():   
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

    node_timeout = 20
    try:
        node_exec_path = os.environ["NODE_PATH"]
        return_code = subprocess.call([node_exec_path, "--config", _node_config_file], timeout=node_timeout)
        return return_code
    except Exception:
        return 1


def client_run_fun():
    work_dir = os.path.join(_work_dir, "client_work_dir")
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)
    os.chdir(work_dir)

    time_to_node_set_up = 5
    time.sleep(time_to_node_set_up)

    pipe = subprocess.run([os.environ["RPC_CLIENT_PATH"], "test", "--host", "127.0.0.1:50051"], capture_output=True)
    
    if b"Test passed" in pipe.stdout and pipe.returncode == 0:
        return 0
    else:
        return 1


def main():
    node_process = mp.Process(target=node_run_fun)
    client_process = mp.Process(target=client_run_fun)
    
    node_process.start()
    client_process.start()

    client_process.join()
    node_process.kill()

    return client_process.exitcode
