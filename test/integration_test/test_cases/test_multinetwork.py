import os
import time
import shutil
import subprocess
import multiprocessing as mp


_work_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test_multitransaction")


def node_run(node_exec_path, address, port, rpc_port):
    if(address != "127.0.0.3"):   
        node_config_file_content = '''
        {
            "net": {
                "listen_addr": "''' + address + ':' + port + '''",
                "public_port": ''' + port + '''
            },
            "rpc": {
                "address": "''' + address + ':' + rpc_port + '''"
            },
            "miner": {
                "threads": 2
            },
            "nodes": [
            ]
        } 
        '''
    else:
        time.sleep(1)
        node_config_file_content = '''
        {
            "net": {
                "listen_addr": "''' + address + ':' + port + '''",
                "public_port": ''' + port + '''
            },
            "rpc": {
                "address": "''' + address + ':' + rpc_port + '''"
            },
            "miner": {
                "threads": 2
            },
            "nodes": [ "127.0.0.1:20202"
            ]
        } 
        '''

    work_dir = os.path.join(_work_dir, "node_work_dir" + address)
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)
    os.chdir(work_dir)

    _node_config_file = os.path.join(work_dir, "config.json")

    with open(_node_config_file, 'w') as node_config:
        node_config.write(node_config_file_content)

    test_timeout = 2
    try:
        return_code = subprocess.run([node_exec_path, "--config", _node_config_file], capture_output=True, timeout=test_timeout)
    except Exception:
        pass
    exit(0)


def main(node_exec_path, rpc_client_exec_path):
    if os.path.exists(_work_dir):
        shutil.rmtree(_work_dir, ignore_errors=True)
    os.makedirs(_work_dir)

    node_process1 = mp.Process(target=node_run, args=[node_exec_path, "127.0.0.1", "20202", "50051"])
    node_process2 = mp.Process(target=node_run, args=[node_exec_path, "127.0.0.2", "20203", "50052"])
    node_process3 = mp.Process(target=node_run, args=[node_exec_path, "127.0.0.3", "20204", "50053"])
    
    node_process1.start()
    node_process2.start()
    node_process3.start()

    node_process1.join()
    node_process1.close()
    node_process2.join()
    node_process2.close()
    node_process3.join()
    node_process3.close()

    return 0
