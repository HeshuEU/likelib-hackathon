import os
import time
import shutil
import subprocess
import multiprocessing as mp


_work_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test_multinetwork")

class NodeInfo:
    def __init__(self, address, port, rpc_port):
        self.address = address
        self.port = port
        self.rpc_port = rpc_port


def node_run(node_exec_path, node_info, another_nodes = []):
    another_nodes_str = ""
    for i in another_nodes:
        another_nodes_str += '"' + i.address + ':' + i.port + '",\n'
    if len(another_nodes_str) != 0:
        another_nodes_str = another_nodes_str[0:len(another_nodes_str) - 2]

    node_config_file_content = '''
        {
            "net": {
                "listen_addr": "''' + node_info.address + ':' + node_info.port + '''",
                "public_port": ''' + node_info.port + '''
            },
            "rpc": {
                "address": "''' + node_info.address + ':' + node_info.rpc_port + '''"
            },
            "miner": {
                "threads": 2
            },
            "nodes": [ 
                ''' + another_nodes_str + '''
            ],
            "database": {
                "path": "likelib/database",
                "clean": false
            }
        } 
        '''

    work_dir = os.path.join(_work_dir, "node_work_dir" + node_info.address)
    if not os.path.exists(work_dir):
        os.makedirs(work_dir)
    os.chdir(work_dir)

    _node_config_file = os.path.join(work_dir, "config.json")

    with open(_node_config_file, 'w') as node_config:
        node_config.write(node_config_file_content)

    time.sleep(len(another_nodes))
    test_timeout = 6
    try:
        node_pipe = subprocess.run([node_exec_path, "--config", _node_config_file], capture_output=True, timeout=test_timeout)
    except Exception:
        pass

    logs_dir = os.path.join(work_dir, "logs")
    log_path = ""
    for file_name in os.listdir(logs_dir):
        if file_name.endswith(".log"):
            log_path = os.path.join(logs_dir, file_name) 
            break
    
    if not os.path.isfile(log_path):
        print(f"The log file for the node " + node_info.address + " does not exist")
        exit(1)
    
    with open(log_path) as log_file:
        log_content = log_file.read()

    for i in another_nodes:
        if (f"Connection established: " + i.address) not in log_content:
            print(f"Node with address " + node_info.address + " did not connection the node with address " + i.address)
            exit(1)

    if node_pipe.returncode != 0:
            #print(node_pipe.stderr)
            print("The node with address " + node_info.address + " that ended up with error")

    exit(0)


def main(node_exec_path, rpc_client_exec_path):
    if os.path.exists(_work_dir):
        shutil.rmtree(_work_dir, ignore_errors=True)
    os.makedirs(_work_dir)

    node_inf1 = NodeInfo("127.0.0.1", "20202", "50051")
    node_inf2 = NodeInfo("127.0.0.2", "20203", "50052")
    node_inf3 = NodeInfo("127.0.0.3", "20204", "50053")
    node_process1 = mp.Process(target=node_run, args=[node_exec_path, node_inf1])
    node_process2 = mp.Process(target=node_run, args=[node_exec_path, node_inf2, [node_inf1]])
    node_process3 = mp.Process(target=node_run, args=[node_exec_path, node_inf3, [node_inf2, node_inf1]])
    
    node_process1.start()
    node_process2.start()
    node_process3.start()

    node_process1.join()
    exit_code = node_process1.exitcode
    node_process1.close()

    node_process2.join()
    exit_code |= node_process2.exitcode
    node_process2.close()

    node_process3.join()
    exit_code |= node_process3.exitcode
    node_process3.close()

    return exit_code
