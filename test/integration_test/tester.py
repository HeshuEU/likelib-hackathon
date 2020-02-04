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
        log_message = ''
        for i in traceback_list:
            if(i.find('TEST_CHECK') > 0):
                log_message = i
                break
        if message and len(message) > 0:
            raise CheckFailedException(f"Check failed: {message}.\n{log_message}")
        else:
            raise CheckFailedException(f"Check failed: {log_message}")


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

        formatter = logging.Formatter(
            '%(asctime)s - %(levelname)s - %(message)s')
        fh.setFormatter(formatter)

        self.logger.addHandler(fh)

    def info(self, message):
        self.logger.info(message)

    def debug(self, message):
        self.logger.debug(message)

    def error(self, message):
        self.logger.error(message)


class NodeRunner:
    DISTRIBUTOR_ADDRESS = '0' * 32
    BUFFER_FILE_NAME = "temp.lock"
    LOG_PREFIX = "NodeRunner"
    CLOSE_TIME_PROCESS_OUT = 2
    running = False

    def __init__(self, node_exec_path, node_config_content, work_dir, logger, start_up_time=1):
        self.logger = logger
        self.work_dir = os.path.abspath(work_dir)
        self.node_exec_path = node_exec_path
        self.logger.debug(
            f"{NodeRunner.LOG_PREFIX} - work directory[{self.work_dir}] node executable file[{self.node_exec_path}]")
        if os.path.exists(self.work_dir):
            shutil.rmtree(self.work_dir, ignore_errors=True)
            self.logger.debug(
                f"{NodeRunner.LOG_PREFIX} - clean up work directory[{self.work_dir}]")
        os.makedirs(self.work_dir)

        self.node_config_file = os.path.join(self.work_dir, "config.json")
        with open(self.node_config_file, 'w') as node_config:
            node_config.write(node_config_content)
        self.logger.debug(
            f"{NodeRunner.LOG_PREFIX} - config saved by path[{self.node_config_file}] with content[{node_config_content}]")

        self.buffer_file = os.path.join(
            self.work_dir, NodeRunner.BUFFER_FILE_NAME)
        self.start_up_time = start_up_time
        self.logger.debug(
            f"{NodeRunner.LOG_PREFIX} - buffer file path[{self.buffer_file}] and start up time[{self.start_up_time}]")

    @property
    def pid(self):
        if self.running:
            return self.process.pid
        else:
            return -1

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
            raise Exception(
                f"{NodeRunner.LOG_PREFIX} - Process already started")

        self.process = subprocess.Popen(
            [self.node_exec_path, "--config", self.node_config_file], cwd=self.work_dir, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        self.logger.info(
            f"{NodeRunner.LOG_PREFIX} - start node(pid:{self.process.pid}) with work directory: {self.work_dir}")

        if self.process.poll() is None:
            self.logger.info(
                f"{NodeRunner.LOG_PREFIX} - running node with work directory {self.work_dir}")
            self.running = True
        else:
            self.logger.info(
                f"{NodeRunner.LOG_PREFIX} - failed running node with work directory:{self.work_dir}")
            self.running = False
            raise Exception(
                f"{NodeRunner.LOG_PREFIX} - Process failed to start")

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
            pid = proc.pid
            self.logger.debug(
                f"{NodeRunner.LOG_PREFIX} - start util process with pid:{pid}")

            # can't read all buffer and wait EOF because process is infinite
            proc.join(1)
            if proc.is_alive():
                proc.kill()
                proc.join()
            exit_code = proc.exitcode
            proc.close()
            self.logger.debug(
                f"{NodeRunner.LOG_PREFIX} - closed util process with pid:{pid}")

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
            raise Exception(
                f"{NodeRunner.LOG_PREFIX} - Buffer file was not found")

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def close(self):
        if self.running:
            pid = self.process.pid
            self.logger.info(
                f"{NodeRunner.LOG_PREFIX} - try to close node with work_dir {self.work_dir}")
            self.process.send_signal(signal.SIGINT)
            try:
                self.process.wait(timeout=NodeRunner.CLOSE_TIME_PROCESS_OUT)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.logger.info(
                    f"{NodeRunner.LOG_PREFIX} - kill node with work_dir {self.work_dir}")
            exit_code = self.process.poll()
            self.running = False
            self.logger.info(
                f"{NodeRunner.LOG_PREFIX} - closed node(exit code:{exit_code}, pid:{pid}) with work_dir {self.work_dir}")


class Client:
    Result = collections.namedtuple('Result', ["success", "message"])
    LOG_PREFIX = "Client"

    def __init__(self, rpc_client_exec_path, work_dir, logger):
        self.logger = logger
        self.work_dir = os.path.abspath(work_dir)
        self.rpc_client_exec_path = rpc_client_exec_path
        self.logger.debug(
            f"{Client.LOG_PREFIX} - work directory[{self.work_dir}] client executable file[{self.rpc_client_exec_path}]")

        if os.path.exists(self.work_dir):
            shutil.rmtree(self.work_dir, ignore_errors=True)
            self.logger.debug(
                f"{Client.LOG_PREFIX} - clean up work directory[{self.work_dir}]")

    def __run(self, *, command, parameters, host_id, timeout):
        if not os.path.exists(self.work_dir):
            os.makedirs(self.work_dir)

        run_commands = [self.rpc_client_exec_path,
                        command, "--host", host_id.connect_rpc_address]
        run_commands.extend(parameters)

        self.logger.info(
            f"{Client.LOG_PREFIX} - {command} start with parameters [{parameters}] to node address {host_id.connect_rpc_address}")
        try:
            pipe = subprocess.run(
                run_commands, cwd=self.work_dir, capture_output=True, timeout=timeout)
        except subprocess.TimeoutExpired:
            message = f"{Client.LOG_PREFIX} - slow command execution {command} with parameters [{parameters}] at node {host_id.connect_rpc_address}"
            self.logger.info(message)
            raise TimeOutException(message)

        if pipe.returncode != 0:
            return Client.Result(not bool(pipe.returncode), pipe.stderr)

        return Client.Result(not bool(pipe.returncode), pipe.stdout)

    def test(self, *, host_id, timeout=1):
        result = self.__run(command="test", parameters=[],
                            host_id=host_id, timeout=timeout)
        parsed_result = self.parse_result(result)
        self.logger.info(
            f"{Client.LOG_PREFIX} - test end to node address {host_id.connect_rpc_address} with result: \"{parsed_result}\"")
        return result

    @staticmethod
    def check_test_result(result):
        if result.success and b"Test passed\n" in result.message:
            return True
        else:
            return False

    def run_check_test(self, *, host_id, timeout=1):
        return self.check_test_result(self.test(host_id=host_id, timeout=timeout))

    def transfer(self, *, from_address, to_address, amount, host_id, wait, timeout=1):
        parameters = ["--from", from_address, "--to",
                      to_address, "--amount", str(amount)]
        result = self.__run(command="transfer",
                            parameters=parameters, host_id=host_id, timeout=timeout)
        parsed_result = self.parse_result(result)
        self.logger.info(
            f"{Client.LOG_PREFIX} - transfer end with parameters [{parameters}] to node {host_id.connect_rpc_address} with result: \"{parsed_result}\"")
        time.sleep(wait)
        return result

    @staticmethod
    def parse_result(result):
        if result is not None:
            return result.message.decode('utf-8').strip()
        else:
            return ""

    @staticmethod
    def check_transfer_result(result):
        if result.success and b"Remote call of transaction -> [Success! Transaction added to queue successfully.]\n" in result.message:
            return True
        else:
            return False

    def run_check_transfer(self, *, from_address, to_address, amount, host_id, wait, timeout=1):
        return self.check_transfer_result(self.transfer(from_address=from_address, to_address=to_address, amount=amount, host_id=host_id, wait=wait, timeout=timeout))

    def get_balance(self, *, address, host_id, timeout=1):
        result = self.__run(command="get_balance", parameters=[
                            "--address", address], host_id=host_id, timeout=timeout)
        parsed_result = self.parse_result(result)
        self.logger.info(
            f"{Client.LOG_PREFIX} - get_balance end for address {address} to node address {host_id.connect_rpc_address} with result: \"{parsed_result}\"")
        return result

    @staticmethod
    def check_get_balance_result(result, target_balance):
        if result.success and (f"Remote call of get_balance -> [{target_balance}]\n").encode('utf8') in result.message:
            return True
        else:
            return False

    def run_check_balance(self, *, address, host_id, target_balance, timeout=1):
        return self.check_get_balance_result(self.get_balance(address=address, host_id=host_id, timeout=timeout), target_balance)


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

            return func(*args, **kargs)

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
        try:
            return_code = registered_test_case_runner(*args, **kargs)
        except Exception as error:
            print(error)
            return_code = 2
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
