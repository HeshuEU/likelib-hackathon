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
import signal
import logging
import re


class CheckFailedException(Exception):
    pass


class TimeOutException(Exception):
    pass


def TEST_CHECK(boolean_value, *, message=""):
    if not boolean_value:

        traceback_list = traceback.format_stack()
        log_message = ""
        for i in range(len(traceback_list)):
            if (traceback_list[i].find('in test_case_exception_wrapper') > 0) and (traceback_list[i].find('return func(*args, **kargs)\n') > 0):
                for log_line in traceback_list[i:-1]:
                    log_message = log_message + log_line
                break
        if message and len(message) > 0:
            raise CheckFailedException(f"Check failed: {message}.\n Trace:\n{log_message}")
        else:
            raise CheckFailedException(f"Check failed.\n Trace:\n{log_message}")


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


class Log:
    def __init__(self, log_file, log_level=logging.DEBUG):
        self.logger = logging.getLogger(str(hash(os.getcwd() + log_file)))
        self.logger.setLevel(log_level)

        fh = logging.FileHandler(os.path.abspath(log_file))
        fh.setLevel(log_level)

        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        fh.setFormatter(formatter)

        self.logger.addHandler(fh)

    def info(self, message):
        self.logger.info(message)
        self.flush()

    def debug(self, message):
        self.logger.debug(message)
        self.flush()

    def error(self, message):
        self.logger.error(message)
        self.flush()

    def flush(self):
        for handler in self.logger.handlers:
            handler.flush()


class NodeTester:
    Result = collections.namedtuple('Result', ["success", "message"])

    DISTRIBUTOR_ADDRESS = '0' * 32

    def __init__(self, node_exec_path, rpc_client_exec_path, node_id, logger, *, 
                    nodes_id_list=[], miner_threads=2,
                    path_to_database="likelib/database", clean_up_database=True):
        self.logger = logger
        self.name = "Node_" + str(node_id.rpc_port)
        self.work_dir = os.path.abspath(self.name)
        self.node_exec_path = node_exec_path
        self.rpc_client_exec_path = rpc_client_exec_path
        self.id = node_id
        self.nodes_id_list = nodes_id_list
        self.miner_threads = miner_threads
        self.path_to_database = path_to_database
        self.logger.debug(f"{self.name} - work directory[{self.work_dir}] node executable file[{self.node_exec_path}]")
        if os.path.exists(self.work_dir):
            shutil.rmtree(self.work_dir, ignore_errors=True)
            self.logger.debug(f"{self.name} - clean up work directory[{self.work_dir}]")
        os.makedirs(self.work_dir)
        self.node_config_file = os.path.join(self.work_dir, "config.json")
        self.__running = False

    @staticmethod
    def __generate_config(current_node_id, miner_threads, nodes_id_list, path_to_database, clean_up_database):
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

    @staticmethod
    def parse_result(result):
        if result is not None:
            return result.message.decode('utf-8').strip()
        else:
            return ""

    def __run_client_command(self, *, command, parameters, timeout):
        run_commands = [self.rpc_client_exec_path, command, "--host", self.id.connect_rpc_address]
        run_commands.extend(parameters)

        self.logger.info(f"{self.name} - client {command} start with parameters {parameters} to node address {self.id.connect_rpc_address}")
        try:
            pipe = subprocess.run(run_commands, cwd=self.work_dir, capture_output=True, timeout=timeout)
        except subprocess.TimeoutExpired:
            message = f"{self.name} - client slow command execution {command} with parameters {parameters} at node {self.id.connect_rpc_address}"
            print(f"{self.name} - is in dead lock. pid {self.pid}, rpc address: {self.id.connect_rpc_address}")
            self.logger.info(message)
            raise TimeOutException(message)

        if pipe.returncode != 0:
            return NodeTester.Result(not bool(pipe.returncode), pipe.stderr)

        return NodeTester.Result(not bool(pipe.returncode), pipe.stdout)

    def test(self, *, timeout=1):
        result = self.__run_client_command(command="test", parameters=[], timeout=timeout)
        self.logger.info(f"{self.name} - test command end at node address {self.id.connect_rpc_address} with result: \"{self.parse_result(result)}\"")
        return result

    @staticmethod
    def check_test_result(result):
        if result.success and b"Test passed\n" in result.message:
            return True
        else:
            return False

    def run_check_test(self, *, timeout=1):
        TEST_CHECK(self.check_test_result(self.test(timeout=timeout)), message=f"fail during connection test to node[{self.name}]")

    def transfer(self, *, from_address, to_address, amount, wait, timeout=1):
        parameters = ["--from", from_address, "--to", to_address, "--amount", str(amount)]
        result = self.__run_client_command(command="transfer", parameters=parameters, timeout=timeout)
        self.logger.info(f"{self.name} - transfer command end with parameters {parameters} to node {self.id.connect_rpc_address} with result: \"{self.parse_result(result)}\"")
        time.sleep(wait)
        return result

    @staticmethod
    def check_transfer_result(result):
        if result.success and b"Remote call of transaction -> [Success! Transaction added to queue successfully.]\n" in result.message:
            return True
        else:
            return False

    def run_check_transfer(self, *, from_address, to_address, amount, wait, timeout=1):
        TEST_CHECK(self.check_transfer_result(self.transfer(from_address=from_address, to_address=to_address, amount=amount, wait=wait, timeout=timeout)), 
                    message=f"fail during transfer to node[{self.name}], from={from_address}, to={to_address}, amount={amount}")

    def get_balance(self, *, address, timeout=1):
        result = self.__run_client_command(command="get_balance", parameters=["--address", address], timeout=timeout)
        self.logger.info(f"{self.name} - get_balance command end for address {address} to node address {self.id.connect_rpc_address} with result: \"{self.parse_result(result)}\"")
        return result

    @staticmethod
    def check_get_balance_result(result, target_balance):
        if result.success and (f"Remote call of get_balance -> [{target_balance}]\n").encode('utf8') in result.message:
            return True
        else:
            return False

    def run_check_balance(self, *, address, target_balance, timeout=1):
        TEST_CHECK(self.check_get_balance_result(self.get_balance(address=address, timeout=timeout), target_balance), 
                    message=f"fail balance check to node[{self.name}], address={address}, target_balance={target_balance}")

    def __write_config(self):
        node_config_content = NodeTester.__generate_config(self.id, self.miner_threads, self.nodes_id_list, self.path_to_database, True)
        with open(self.node_config_file, 'w') as node_config:
            node_config.write(node_config_content)
        self.logger.debug(f"{self.name} - config saved by path[{self.node_config_file}] with content[{node_config_content}]")

    def append_node_id(self, node_id):
        self.nodes_id_list.append(node_id)

    @property
    def pid(self):
        if self.__running:
            return self.process.pid
        else:
            return -1

    def start_node(self, start_up_time=1):
        if self.__running:
            raise Exception(f"{self.name} - Process already started")

        self.__write_config()
        self.process = subprocess.Popen([self.node_exec_path, "--config", self.node_config_file], cwd=self.work_dir, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        self.logger.debug(f"{self.name} - start node(pid:{self.process.pid}) with work directory: {self.work_dir}")

        if self.process.poll() is None:
            self.logger.info(f"{self.name} - running node with work directory {self.work_dir}")
            self.__running = True
        else:
            self.logger.info(f"{self.name} - failed running node with work directory:{self.work_dir}")
            self.__running = False
            raise Exception(f"{self.name} - Process failed to start")

        time.sleep(start_up_time)

    def __enter__(self):
        self.start_node(2)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def close(self):
        if self.__running:
            pid = self.process.pid
            self.logger.info(f"{self.name} - try to close node with work_dir {self.work_dir}")
            self.process.send_signal(signal.SIGINT)
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.logger.info(f"{self.name} - kill node with work_dir {self.work_dir}")
            exit_code = self.process.poll()
            self.__running = False
            self.logger.info(f"{self.name} - closed node(exit code:{exit_code}, pid:{pid}) with work_dir {self.work_dir}")


class NodePoll:
    def __init__(self):
        self.nodes = list()
        self.last = None

    def append(self, node):
        self.nodes.append(node)
        self.last = node

    def __enter__(self):
        return self

    def start_nodes(self, waiting_time=1):
        for node in self.nodes:
            node.start_node(waiting_time)

    @property
    def ids(self):
        return [node.id for node in self.nodes]

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def __getitem__(self, key):
        return self.nodes[key]

    def close(self):
        for node in self.nodes:
            node.close()

    def __iter__(self):
        def iter_fn(nodes):
            for node in nodes:
                yield node

        return iter_fn(self.nodes)

    def __len__(self): 
        return len(self.nodes)

    @staticmethod
    def create_pool_of_nodes_one_by_one(node_exec_path, rpc_client_exec_path, start_sync_port, start_rpc_port, count_nodes, logger):
        pool = NodePoll() 
        pool.append(NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=start_sync_port, rpc_port=start_rpc_port), logger))

        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(NodeTester(node_exec_path, rpc_client_exec_path, NodeId(sync_port=curent_sync_port, rpc_port=curent_rpc_port), logger, nodes_id_list=[pool.last.id, ]))
        return pool


__enabled_tests = dict()
__disabled_tests = dict()


def test_case(registration_test_case_name=None, disable=False):
    def test_case_registrator(func):
        if registration_test_case_name:
            test_name = registration_test_case_name
        else:
            test_name = func.__name__

        __test_case_work_dir = os.path.join(os.getcwd(), test_name)

        def test_case_runner(*args, **kargs):
            # change work directory for test
            if os.path.exists(__test_case_work_dir):
                shutil.rmtree(__test_case_work_dir, ignore_errors=True)
            os.makedirs(__test_case_work_dir)
            os.chdir(__test_case_work_dir)

            def test_case_exception_wrapper(*args, **kargs):
                try:
                    return func(*args, **kargs)
                except Exception as error:
                    print(error)
                    return 2

            return test_case_exception_wrapper(*args, **kargs)

        if test_name in __enabled_tests.keys() or test_name in __disabled_tests.keys():
            raise Exception(f"Test with this name[{test_name}] is exists")

        if disable:
            print(f"Registered test case {test_name} is disabled")
            __disabled_tests[test_name] = test_case_runner
        else:
            print(f"Registered test case {test_name} is enabled")
            __enabled_tests[test_name] = test_case_runner

        return test_case_runner
    return test_case_registrator


def run_registered_test_cases(pattern, *args, **kargs):
    success_tests = 0
    failed_tests = 0
    skipped_tests = 0

    try:
        matcher = re.compile(pattern)
    except Exception as e:
        print(f"Invalid pattern: {e}")
        exit(2)

    print(
        f"Enabled tests: {len(__enabled_tests)}. Disabled tests: {len(__disabled_tests)}.")

    for registered_test_case_name in __enabled_tests:
        if matcher.match(registered_test_case_name) is None:
            skipped_tests += 1
            continue

        registered_test_case_runner = __enabled_tests[registered_test_case_name]

        print(f"Test case {registered_test_case_name} started.")

        test_case_start_time = datetime.datetime.now()
        return_code = registered_test_case_runner(*args, **kargs)
        test_case_execute_time = datetime.datetime.now() - test_case_start_time

        if return_code == 0:
            print(
                f"Test case {registered_test_case_name} success. Execute time: {test_case_execute_time}.")
            success_tests += 1
        else:
            print(
                f"Test case {registered_test_case_name} failed. Execute time: {test_case_execute_time}.")
            failed_tests += 1

    all_tests = success_tests + failed_tests + skipped_tests
    print(
        f"All test cases: {all_tests}. Passed tests: {success_tests}. Failed tests: {failed_tests}. Skipped tests: {skipped_tests}.")
    return failed_tests
