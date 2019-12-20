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
        print("Check failed:", message)
        traceback.print_stack()
        exit(1)


def TEST_CHECK_EQUAL(left, right, *, message=""):
    TEST_CHECK(left == right, message=message)


def TEST_CHECK_NOT_EQUAL(left, right, *, message=""):
    TEST_CHECK(left != right, message=message)


class NodeRunner:
    running = False

    def __init__(self, node_exec_path, work_dir):
        self.work_dir = os.path.abspath(work_dir)
        self.node_exec_path = node_exec_path

    @staticmethod
    def generate_config(node_work_dir="", listen_address="0.0.0.0",
                        net_public_port=20021, rpc_listening_port=50051, miner_threads=2, nodes_list=[],
                        path_to_database="likelib/database", clean_up_database=True):

        config = {"net": {"listen_addr": f"{listen_address}:{net_public_port}",
                          "public_port":  net_public_port},
                  "rpc": {"address": f"{listen_address}:{rpc_listening_port}"},
                  "miner": {"threads": miner_threads},
                  "nodes": nodes_list,
                  "database": {"path": path_to_database,
                               "clean": clean_up_database}
                  }
        return json.dumps(config)

    @staticmethod
    def _runner(work_dir, node_exec_path, node_config_file, time_out):
        os.chdir(work_dir)
        try:
            print("Debug message:", "sterted")
            node_pipe = subprocess.run(
                [node_exec_path, "--config", node_config_file], capture_output=True, timeout=time_out)
            print("Debug message:", "stoped")
            print("Debug message:", node_pipe.stdout)
            print("Debug message:", node_pipe.stderr)
        except Exception:
            pass

    def start(self, node_config_content, time_out=15):

        if not os.path.exists(self.work_dir):
            os.makedirs(self.work_dir)
        os.chdir(self.work_dir)

        node_config_file = "config.json"
        with open(node_config_file, 'w') as node_config:
            node_config.write(node_config_content)

        self.process = mp.Process(target=NodeRunner._runner, args=[
                                  self.work_dir, self.node_exec_path, node_config_file, time_out])
        self.process.start()
        self.running = True
        time.sleep(5)

    def check(self):
        # todo run parallel process to check output
        if self.running:
            self.process.join()
            exit_code = self.process.exitcode
            self.process.close()
            self.running = False
            return exit_code

    def clean_up(self):
        if not self.running:
            if os.path.exists(self.work_dir):
                shutil.rmtree(self.work_dir, ignore_errors=True)


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
        print("Client | Debug message: clean up call")
        if os.path.exists(self.work_dir):
            shutil.rmtree(self.work_dir, ignore_errors=True)


class NetworkBuilder:
    pass


if __name__ == "__main__":
    node_exec_path = "/home/siarhei_sadouski/Documents/likelib/cmake-build-debug/src/node/node"
    rpc_client_exec_path = "/home/siarhei_sadouski/Documents/likelib/cmake-build-debug/src/rpc-client/rpc-client"
    work_dir = os.path.join(os.path.dirname(
        os.path.abspath(__file__)), "test_helper")

    def test_connection(work_dir, node_exec_path, rpc_client_exec_path):
        node = NodeRunner(node_exec_path, os.path.join(work_dir, "node"))
        node.start(node.generate_config())

        client = Client(rpc_client_exec_path,
                              os.path.join(work_dir, "client"))
        TEST_CHECK(client.run_check_test(host_port=50051))

        return_code = node.check()

        node.clean_up()
        client.clean_up()

    def test_transfer(work_dir, node_exec_path, rpc_client_exec_path):
        node = NodeRunner(node_exec_path, os.path.join(work_dir, "node"))

        client = Client(rpc_client_exec_path,
                              os.path.join(work_dir, "client"))

        node.start(node.generate_config())

        TEST_CHECK(client.run_check_test(host_port=50051))

        target_address = "11111111111111111111111111111111"

        TEST_CHECK(client.run_check_balance(address=target_address,
                                            host_port=50051, target_balance=0))

        from_address = "00000000000000000000000000000000"

        TEST_CHECK(client.run_check_transfer(from_address=from_address,
                                             to_address=target_address, amount=333, host_port=50051, wait=2))

        TEST_CHECK(client.run_check_balance(address=target_address,
                                            host_port=50051, target_balance=333))

        return_code = node.check()

        node.clean_up()
        client.clean_up()

    test_connection(os.path.join(work_dir, "test_connection"),
                    node_exec_path, rpc_client_exec_path)
    test_transfer(os.path.join(work_dir, "test_transfer"),
                  node_exec_path, rpc_client_exec_path)
