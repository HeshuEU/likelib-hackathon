import os
import time
import shutil
import subprocess
import multiprocessing as mp


_work_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test_transaction")


def create_and_check_transaction(rpc_client_exec_path, host_address, from_address, to_address, amount, final_amount):
    pipe = subprocess.run([rpc_client_exec_path, "transfer", "--host", host_address, "--from", from_address, "--to", to_address, "--amount", amount], capture_output=True)

    if pipe.returncode != 0:
        print(pipe.stderr)
        exit(1)
    
    transfer_time_out = 3
    time.sleep(transfer_time_out)

    pipe = subprocess.run([rpc_client_exec_path, "get_balance", "--host", host_address, "--address", to_address], capture_output=True)
    if final_amount.encode() not in pipe.stdout or pipe.returncode != 0:
        print(pipe.stderr)
        exit(1)



def node_run_fun(node_exec_path):   
    node_config_file_content = '''
    {
        "net": {
            "listen_addr": "127.0.0.1:20202",
            "public_port": 20202
        },
        "rpc": {
            "address": "127.0.0.1:50051"
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

    test_timeout = 15
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

    time_to_node_set_up = 3
    time.sleep(time_to_node_set_up)

    host_address = "127.0.0.1:50051"
    
    create_and_check_transaction(rpc_client_exec_path, host_address, "00000000000000000000000000000000", "Shisha", "1000", "1000")
    #create_and_check_transaction(rpc_client_exec_path, host_address, "Shisha", "Andre", "670", "670")  #isn't working right now, waiting 
    #create_and_check_transaction(rpc_client_exec_path, host_address, "Andre", "Shisha", "100", "100")  #for the storage branch to be filled

    exit(0)


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
