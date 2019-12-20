import os
import time
import shutil
import json
import subprocess
import multiprocessing as mp
import collections
import traceback


LOCAL_HOST_IP_ADDRESS = "127.0.0.1"


def TEST_CHECK(boolean_value, *, message=""):
    if not boolean_value:
        traceback.print_stack()
        raise Exception("Check failed:" + message)


def TEST_CHECK_EQUAL(left, right, *, message=""):
    TEST_CHECK(left == right, message=message)


def TEST_CHECK_NOT_EQUAL(left, right, *, message=""):
    TEST_CHECK(left != right, message=message)


class NodeId:
    def __init__(self, sync_port, rpc_port, listening_addres="0.0.0.0", absolute_address="127.0.0.1"):
        self.listening_addres = listening_addres
        self.absolute_address = absolute_address
        self.sync_port = sync_port
        self.rpc_port = rpc_port

    @property
    def listen_sync_address(self):
        return f"{self.listening_addres}:{self.sync_port}"

    @property
    def connect_sync_address(self):
        return f"{self.absolute_address}:{self.sync_port}"

    @property
    def listen_rpc_address(self):
        return f"{self.listening_addres}:{self.rpc_port}"

    @property
    def connect_rpc_address(self):
        return f"{self.absolute_address}:{self.rpc_port}"


class NodeRunner:
    running = False

    def __init__(self, node_exec_path, work_dir):
        self.work_dir = os.path.abspath(work_dir)
        self.node_exec_path = node_exec_path
        self.clean_up()

    @staticmethod
    def generate_config(node_work_dir="", *, current_node_info=NodeId(20203, 50051), miner_threads=2, nodes_infos_list=[],
                        path_to_database="likelib/database", clean_up_database=True):

        config = {"net": {"listen_addr": current_node_info.listen_sync_address,
                          "public_port": current_node_info.sync_port},
                  "rpc": {"address": current_node_info.listen_rpc_address},
                  "miner": {"threads": miner_threads},
                  "nodes": [node_info.connect_sync_address for node_info in nodes_infos_list],
                  "database": {"path": path_to_database,
                               "clean": clean_up_database}
                  }
        return json.dumps(config)

    def start(self, node_config_content, start_up_time=2):
        if self.running:
            raise Exception("Process already started")

        if not os.path.exists(self.work_dir):
            os.makedirs(self.work_dir)
        os.chdir(self.work_dir)

        node_config_file = "config.json"
        with open(node_config_file, 'w') as node_config:
            node_config.write(node_config_content)
        print("Node | Debug message: config content:", node_config_content)
        self.process = subprocess.Popen(
            [self.node_exec_path, "--config", node_config_file], stderr=subprocess.PIPE, stdout=subprocess.PIPE)

        if self.process.poll() is None:
            print("Node | Debug message: process started")
            self.running = True
        else:
            print("Node | Debug message: process failed to start")
            self.running = False

        time.sleep(start_up_time)

    # def check(self):
    #     if self.running:
    #         print("Process stdout:")
    #         while True:
    #             lines = self.process.stderr.read()
    #             print(line.rstrip())
    #     else:
    #         raise Exception("process is not running")

    def close(self):
        self.process.kill()
        exit_code = self.process.poll()
        self.running = False
        self.clean_up()
        print("Node | Debug message: process closed")

    def clean_up(self):
        if not self.running:
            if os.path.exists(self.work_dir):
                shutil.rmtree(self.work_dir, ignore_errors=True)
        else:
            print("Warning: call clean_up during node work")


class Client:
    Result = collections.namedtuple('Result', ["success", "message"])

    def __init__(self, rpc_client_exec_path, work_dir):
        self.work_dir = os.path.abspath(work_dir)
        self.rpc_client_exec_path = rpc_client_exec_path
        self.clean_up()

    def __run(self, *, command, parameters, host_port, host_ip):
        if not os.path.exists(self.work_dir):
            os.makedirs(self.work_dir)
        os.chdir(self.work_dir)

        run_commands = [self.rpc_client_exec_path,
                        command, "--host", f"{host_ip}:{host_port}"]
        run_commands.extend(parameters)

        print("Client | Debug message: call string", run_commands)
        pipe = subprocess.run(run_commands, capture_output=True)

        if pipe.returncode != 0:
            return Client.Result(not bool(pipe.returncode), pipe.stderr)

        return Client.Result(not bool(pipe.returncode), pipe.stdout)

    def test(self, *, host_port, host_ip=LOCAL_HOST_IP_ADDRESS):
        return self.__run(command="test", parameters=[], host_port=host_port, host_ip=host_ip)

    @staticmethod
    def check_test_result(result):
        print("Client | Debug message:", result)
        if result.success and b"Test passed\n" in result.message:
            return True
        else:
            print("test check failed:", result.message)
            return False

    def run_check_test(self, *, host_port, host_ip=LOCAL_HOST_IP_ADDRESS):
        return self.check_test_result(self.test(host_port=host_port, host_ip=host_ip))

    def transfer(self, *, from_address, to_address, amount, host_port, wait, host_ip=LOCAL_HOST_IP_ADDRESS):
        result = self.__run(command="transfer", parameters=[
                            "--from", from_address, "--to", to_address, "--amount", str(amount)], host_port=host_port, host_ip=host_ip)
        time.sleep(wait)
        return result

    @staticmethod
    def check_transfer_result(result):
        print("Client | Debug message:", result)
        if result.success and b"Remote call of transaction -> [likelib]\n" in result.message:
            return True
        else:
            print("transfer check failed:", result.message)
            return False

    def run_check_transfer(self, *, from_address, to_address, amount, host_port, wait, host_ip=LOCAL_HOST_IP_ADDRESS):
        return self.check_transfer_result(self.transfer(from_address=from_address, to_address=to_address, amount=amount, host_port=host_port, wait=wait, host_ip=host_ip))

    def get_balance(self, *, address, host_port, host_ip=LOCAL_HOST_IP_ADDRESS):
        return self.__run(command="get_balance", parameters=["--address", address], host_port=host_port, host_ip=host_ip)

    @staticmethod
    def check_get_balance_result(result, target_balance):
        print("Client | Debug message:", result)
        if result.success and (f"Remote call of get_balance -> [{target_balance}]\n").encode('utf8') in result.message:
            return True
        else:
            print("get_balance check failed:", result.message)
            return False

    def run_check_balance(self, *, address, host_port, target_balance, host_ip=LOCAL_HOST_IP_ADDRESS):
        return self.check_get_balance_result(self.get_balance(address=address, host_port=host_port, host_ip=host_ip), target_balance)

    def clean_up(self):
        if os.path.exists(self.work_dir):
            shutil.rmtree(self.work_dir, ignore_errors=True)


if __name__ == "__main__":
    node_exec_path = "/home/siarhei_sadouski/Documents/likelib/cmake-build-debug/src/node/node"
    rpc_client_exec_path = "/home/siarhei_sadouski/Documents/likelib/cmake-build-debug/src/rpc-client/rpc-client"
    work_dir = os.path.join(os.path.dirname(
        os.path.abspath(__file__)), "test_helper")

    def test_connection(work_dir, node_exec_path, rpc_client_exec_path):
        node = NodeRunner(node_exec_path, os.path.join(work_dir, "node"))
        node_info = NodeId(sync_port=20204, rpc_port=50054)
        node.start(node.generate_config(current_node_info=node_info))

        client = Client(rpc_client_exec_path,
                        os.path.join(work_dir, "client"))
        TEST_CHECK(client.run_check_test(
            host_port=node_info.rpc_port, host_ip=node_info.absolute_address))
        node.close()

    test_connection(os.path.join(work_dir, "test_connection"),
                    node_exec_path, rpc_client_exec_path)

    def test_transfer(work_dir, node_exec_path, rpc_client_exec_path):
        node = NodeRunner(node_exec_path, os.path.join(work_dir, "node"))

        node_info = NodeId(sync_port=20204, rpc_port=50054)

        node.start(node.generate_config(current_node_info=node_info))

        client = Client(rpc_client_exec_path,
                        os.path.join(work_dir, "client"))

        TEST_CHECK(client.run_check_test(
            host_port=node_info.rpc_port, host_ip=node_info.absolute_address))

        target_address = "11111111111111111111111111111111"

        TEST_CHECK(client.run_check_balance(address=target_address, host_ip=node_info.absolute_address,
                                            host_port=node_info.rpc_port, target_balance=0))

        from_address = "00000000000000000000000000000000"
        amount = 333
        transaction_wait = 2
        TEST_CHECK(client.run_check_transfer(from_address=from_address, host_ip=node_info.absolute_address,
                                             to_address=target_address, amount=amount, host_port=node_info.rpc_port, wait=transaction_wait))

        TEST_CHECK(client.run_check_balance(address=target_address, host_ip=node_info.absolute_address,
                                            host_port=node_info.rpc_port, target_balance=amount))
        node.close()

    test_transfer(os.path.join(work_dir, "test_transfer"),
                  node_exec_path, rpc_client_exec_path)
