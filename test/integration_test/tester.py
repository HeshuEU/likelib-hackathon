import os
import time
import shutil
import json
import signal
import datetime
import subprocess
import multiprocessing as mp
import collections
import traceback
import logging


def TEST_CHECK(boolean_value, *, message=""):
    if not boolean_value:
        traceback_list = traceback.format_stack()
        log_message = ''
        for i in traceback_list:
            if(i.find('TEST_CHECK') > 0):
                log_message = i
                break
        raise Exception("Check failed: " + message + '\n' + log_message)


def TEST_CHECK_EQUAL(left, right, *, message=""):
    TEST_CHECK(left == right, message=message)


def TEST_CHECK_NOT_EQUAL(left, right, *, message=""):
    TEST_CHECK(left != right, message=message)

# TODO: implement logger insteed print


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

class Log:

    def __init__(self, log_name):
        self.logger = logging.getLogger(log_name)
        self.logger.setLevel(logging.DEBUG)
        fh = logging.FileHandler(os.path.abspath(log_name))
        fh.setLevel(logging.DEBUG)
        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        fh.setFormatter(formatter)
        self.logger.addHandler(fh)

    def info(self, message):
        self.logger.info(message)


class NodeRunner:
    BUFFER_FILE_NAME = "temp.lock"
    running = False

    def __init__(self, node_exec_path, node_config_content, work_dir, logger, start_up_time=2):
        self.logger = logger
        self.work_dir = os.path.abspath(work_dir)
        # print("Node | Debug message: work dir:", self.work_dir)
        self.node_exec_path = node_exec_path
        if os.path.exists(self.work_dir):
            shutil.rmtree(self.work_dir, ignore_errors=True)
        os.makedirs(self.work_dir)

        self.node_config_file = os.path.join(self.work_dir, "config.json")
        with open(self.node_config_file, 'w') as node_config:
            node_config.write(node_config_content)

        # print("Node | Debug message: config content:", node_config_content)
        self.buffer_file = os.path.join(
            self.work_dir, NodeRunner.BUFFER_FILE_NAME)
        self.start_up_time = start_up_time

    @staticmethod
    def generate_config(*, current_node_id=NodeId(20203, 50051), miner_threads=2, nodes_id_list=[],
                        path_to_database="likelib/database", clean_up_database=True):

        config = {"net": {"listen_addr": current_node_id.listen_sync_address,
                          "public_port": current_node_id.sync_port},
                  "rpc": {"address": current_node_id.listen_rpc_address},
                  "miner": {"threads": miner_threads},
                  "nodes": [node_id.connect_sync_address for node_id in nodes_id_list],
                  "database": {"path": path_to_database,
                               "clean": clean_up_database},
                  "keys": {"public_path": "rsa.pub",
                           "private_path": "rsa"}
                  }
        return json.dumps(config)

    def start(self):
        if self.running:
            raise Exception("Process already started")
        
        self.logger.info(f"start node with work directory {self.work_dir}")
        self.process = subprocess.Popen(
            ["valgrind", self.node_exec_path, "--config", self.node_config_file], cwd=self.work_dir, stderr=subprocess.PIPE, stdout=subprocess.PIPE)

        if self.process.poll() is None:
            self.logger.info(f"running node with work directory {self.work_dir}")
            # print("Node | Debug message: process started")
            self.running = True
        else:
            self.logger.info(f"failed running node with work directory {self.work_dir}")
            # print("Node | Debug message: process failed to start")
            self.running = False
            raise Exception("Process failed to start")

        time.sleep(self.start_up_time)

    def _get_buffer(self):
        if self.running:
            def __check_log(pipe, buffer_file_name):
                with open(buffer_file_name, "at") as temp_file:
                    while True:
                        temp_file.write(pipe.readline().decode("utf8"))
                        temp_file.flush()

            proc = mp.Process(target=__check_log, args=[
                              self.process.stderr, self.buffer_file])
            proc.start()
            # can't read all buffer and wait EOF, but process is infinite
            proc.join(1)
            if proc.is_alive():
                proc.kill()
                proc.join()
            exit_code = proc.exitcode
            proc.close()

    def check(self, check_line_fun):
        self._get_buffer()
        if os.path.exists(self.buffer_file):
            exit_value = False
            with open(self.buffer_file, "rt") as temp_file:
                while True:
                    line = temp_file.readline()
                    if not line:
                        break
                    if check_line_fun(line):
                        exit_value = True
            return exit_value
        else:
            raise Exception("Buffer file was not found")

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def close(self):
        if self.running:
            self.logger.info(f"close node with work_dir {self.work_dir}")
            self.process.send_signal(signal.SIGINT)
            time.sleep(0.2)
            exit_code = self.process.poll()
            self.running = False
            # print("Node | Debug message: process closed")


class Client:
    Result = collections.namedtuple('Result', ["success", "message"])

    def __init__(self, rpc_client_exec_path, work_dir, logger):
        self.logger = logger
        self.work_dir = os.path.abspath(work_dir)
        # print("Client | Debug message: work dir:", self.work_dir)
        self.rpc_client_exec_path = rpc_client_exec_path
        if os.path.exists(self.work_dir):
            shutil.rmtree(self.work_dir, ignore_errors=True)

    def __run(self, *, command, parameters, host_id):
        if not os.path.exists(self.work_dir):
            os.makedirs(self.work_dir)

        run_commands = [self.rpc_client_exec_path,
                        command, "--host", host_id.connect_rpc_address]
        run_commands.extend(parameters)

        # print("Client | Debug message: call string", run_commands)
        try:
            self.logger.info(f"{command} start with parameters " + f"{parameters} --to_node {host_id.listen_sync_address}")
            pipe = subprocess.run(
                run_commands, cwd=self.work_dir, capture_output=True, timeout=15)
        except subprocess.TimeoutExpired:
            self.logger.info(f"slow command execution {command} {parameters} with node {host_id.listen_sync_address}")
            traceback_list = traceback.format_stack()
            log_message = ''
            for i in traceback_list:
                if(i.find('TEST_CHECK') > 0):
                    log_message = i
                    break
            raise Exception(f"slow command execution {command} {parameters} with node {host_id.listen_sync_address}\n{log_message}")

        if pipe.returncode != 0:
            return Client.Result(not bool(pipe.returncode), pipe.stderr)

        return Client.Result(not bool(pipe.returncode), pipe.stdout)

    def test(self, *, host_id):
        result = self.__run(command="test", parameters=[], host_id=host_id)
        self.logger.info("test end with parameters []" + f"--to_node {host_id.listen_sync_address} with result {result.message.decode('utf-8')}")
        return result

    @staticmethod
    def check_test_result(result):
        # print("Client | Debug message:", result)
        if result.success and b"Test passed\n" in result.message:
            return True
        else:
            # print("test check failed:", result.message)
            return False

    def run_check_test(self, *, host_id):
        return self.check_test_result(self.test(host_id=host_id))

    def transfer(self, *, from_address, to_address, amount, host_id, wait):
        result = self.__run(command="transfer", parameters=[
                            "--from", from_address, "--to", to_address, "--amount", str(amount)], host_id=host_id)
        self.logger.info("transfer end with parameters " + f" ['--from', '{from_address}', '--to', '{to_address}', '--amount' '{amount}'] --to_node {host_id.listen_sync_address} with result {result.message.decode('utf-8')}")
        time.sleep(wait)
        return result

    @staticmethod
    def check_transfer_result(result):
        # print("Client | Debug message:", result)
        if result.success and b"Remote call of transaction -> [Success! Transaction added to queue successfully.]\n" in result.message:
            return True
        else:
            print("transfer check failed:", result.message.decode('utf-8'))
            return False

    def run_check_transfer(self, *, from_address, to_address, amount, host_id, wait):
        return self.check_transfer_result(self.transfer(from_address=from_address, to_address=to_address, amount=amount, host_id=host_id, wait=wait))

    def get_balance(self, *, address, host_id):
        result = self.__run(command="get_balance", parameters=["--address", address], host_id=host_id)
        self.logger.info("get_balance end with parameters " + f" ['--address', '{address}'] --to_node {host_id.listen_sync_address} with result {result.message.decode('utf-8')}")

        return result

    @staticmethod
    def check_get_balance_result(result, target_balance):
        # print("Client | Debug message:", result)
        if result.success and (f"Remote call of get_balance -> [{target_balance}]\n").encode('utf8') in result.message:
            return True
        else:
            print("get_balance check failed:", result.message.decode('utf-8'))
            return False

    def run_check_balance(self, *, address, host_id, target_balance):
        self.logger.info(f"check balance in address {address} with balance {target_balance} to node {host_id.listen_sync_address}")
        return self.check_get_balance_result(self.get_balance(address=address, host_id=host_id), target_balance)


__registered_tests = dict()


def test_case(registration_test_case_name=""):
    def test_case_registrator(func):
        if registration_test_case_name:
            test_name = registration_test_case_name
        else:
            test_name = func.__name__
        print(f"Registered test case: {test_name}")

        __test_case_work_dir = os.path.join(os.getcwd(), test_name)

        def test_case_runner(*args, **kargs):
            if os.path.exists(__test_case_work_dir):
                shutil.rmtree(__test_case_work_dir, ignore_errors=True)
            os.makedirs(__test_case_work_dir)
            os.chdir(__test_case_work_dir)

            return func(*args, **kargs)

        if test_name in __registered_tests.keys():
            raise Exception("Test with this name is exists")
        __registered_tests[test_name] = test_case_runner

        return test_case_runner
    return test_case_registrator


# TODO: add regex to run specific test cases
def run_registered_test_cases(*args, **kargs):
    success_tests = 0
    failed_tests = 0

    for registered_test_case_name in __registered_tests:
        registered_test_case_runner = __registered_tests[registered_test_case_name]

        print(f"Test case [{registered_test_case_name}] started.")

        test_case_start_time = datetime.datetime.now()
        try:
            return_code = registered_test_case_runner(*args, **kargs)
        except Exception as error:
            print(f"Catch unexpected exception: {error}")
            return_code = 2
        test_case_execute_time = datetime.datetime.now() - test_case_start_time

        if return_code == 0:
            print(
                f"Test case [{registered_test_case_name}] success. Execute time: {test_case_execute_time}.")
            success_tests += 1
        else:
            print(
                f"Test case [{registered_test_case_name}] failed. Execute time: {test_case_execute_time}.")
            failed_tests += 1

    all_tests = success_tests + failed_tests
    print(
        f"Started test cases: {all_tests}. Passed tests: {success_tests}. Failed tests: {failed_tests}")
    return failed_tests
