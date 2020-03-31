import os
import re
import time
import json
import shutil
import signal
import logging
import datetime
import traceback
import subprocess


# exceptions
class CheckFailedException(Exception):
    pass


class TimeOutException(Exception):
    pass


# test case checks
def TEST_CHECK(boolean_value, *, message=""):
    if not boolean_value:
        traceback_list = traceback.format_stack()
        log_message = ""
        for i in range(len(traceback_list)):
            if (traceback_list[i].find('in test_case_exception_wrapper') > 0) and (
                    traceback_list[i].find('return func(env, logger)\n') > 0):
                for log_line in traceback_list[i + 1:-1]:
                    log_message = log_message + log_line
                break
        if message and len(message) > 0:
            raise CheckFailedException(
                f"Check failed: {message}.\n Trace:\n{log_message}")
        else:
            raise CheckFailedException(
                f"Check failed.\n Trace:\n{log_message}")


def TEST_CHECK_EQUAL(left, right, *, message=""):
    TEST_CHECK(left == right, message=message)


def TEST_CHECK_NOT_EQUAL(left, right, *, message=""):
    TEST_CHECK(left != right, message=message)


# logger
class Logger:
    def __init__(self, log_name, log_level=logging.DEBUG):
        self.logger = logging.getLogger(log_name)
        self.logger.setLevel(log_level)

        fh = logging.FileHandler(os.path.abspath('test_runner.log'))
        fh.setLevel(log_level)

        formatter = logging.Formatter(
            '%(asctime)s - %(levelname)s - %(message)s')
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


# nodes api
def _signature_checker(signature_status):
    return signature_status == "verified"


class _BlockParser:
    _BLOCK_RE = {
        'block_hash': (re.compile(r'Block hash (?P<block_hash>.*)'), str),
        'depth': (re.compile(r'Depth: (?P<depth>\d+)'), int),
        'timestamp': (re.compile(r'Timestamp: (?P<timestamp>\d+)'), int),
        'coinbase': (re.compile(r'Coinbase: (?P<coinbase>.*)'), str),
        'previous_block_hash': (re.compile(r'Previous block hash: (?P<previous_block_hash>.*)'), str),
        'number_of_transactions': (re.compile(r'Number of transactions: (?P<number_of_transactions>\d+)'), int)
    }

    _TX_RE = {
        'number': (re.compile(r'Transaction #(?P<number>\d+)'), int),
        'type': (re.compile(r'Type: (?P<type>.*)'), str),
        'from': (re.compile(r'From: (?P<from>.*)'), str),
        'to': (re.compile(r'To: (?P<to>.*)'), str),
        'value': (re.compile(r'Value: (?P<value>\d+)'), int),
        'fee': (re.compile(r'Fee: (?P<fee>\d+)'), int),
        'timestamp': (re.compile(r'Timestamp: (?P<timestamp>\d+)'), int),
        'data': (re.compile(r'Data: (?P<data>.*)'), str),
        'signature': (re.compile(r'Signature: (?P<signature>.*)'), _signature_checker)
    }

    @staticmethod
    def _parse_line(rx_dict, line):
        for key, rx in rx_dict.items():
            match = rx[0].search(line)
            if match:
                return key, match
        return None, None

    @staticmethod
    def _add(result, key, match, target, target_type):
        if key == target:
            result[target] = target_type(match.group(target))

    @staticmethod
    def _parse_transaction(lines):
        result = dict()
        for line in lines:
            key, match = _BlockParser._parse_line(_BlockParser._TX_RE, line)
            for target in _BlockParser._TX_RE:
                _BlockParser._add(result, key, match, target,
                                  _BlockParser._TX_RE[key][1])
        if len(result) != len(_BlockParser._TX_RE):
            return None
        return result

    @staticmethod
    def parse(text):
        result = dict()
        lines = text.split('\n')
        header_lines = lines[0:len(_BlockParser._BLOCK_RE)]
        for line in header_lines:
            key, match = _BlockParser._parse_line(_BlockParser._BLOCK_RE, line)
            for target in _BlockParser._BLOCK_RE:
                _BlockParser._add(result, key, match, target,
                                  _BlockParser._BLOCK_RE[key][1])
        if len(result) != len(_BlockParser._BLOCK_RE):
            return None

        result["transactions"] = list()
        for tx_number in range(result["number_of_transactions"]):
            tx_lines = lines[len(_BlockParser._BLOCK_RE) + tx_number *
                             len(_BlockParser._TX_RE):len(_BlockParser._BLOCK_RE) + (tx_number + 1) * len(
                _BlockParser._TX_RE)]
            result["transactions"].append(
                _BlockParser._parse_transaction(tx_lines))

        return result


class _InfoParser:
    _TARGET = "block_hash"
    _RE_PARSER = re.compile(r'Top block hash: (?P<block_hash>.*)')

    @staticmethod
    def parse(text):
        match = _InfoParser._RE_PARSER.search(text)
        if not match:
            return None
        return match.group(_InfoParser._TARGET)


class _BalanceParser:
    _TARGET = "balance"
    _RE_PARSER = re.compile(r'balance of (?P<address>.*) is (?P<balance>\d+)')

    @staticmethod
    def parse(text):
        match = _BalanceParser._RE_PARSER.search(text)
        if not match:
            return None
        return int(match.group(_BalanceParser._TARGET))


class _TestParser:
    @staticmethod
    def parse(text):
        return text == "Test passed\n"


class DeployedContract:
    def __init__(self, address=None, gas_left=None):
        self.address = address
        self.gas_left = gas_left


class _PushContractParser:
    _ADDRESS_TARGET = "address"
    _GAS_LEFT_TARGET = "gas_left"
    _RE_PARSER = re.compile(
        r'Remote call of creation smart contract success -> \[Contract was successfully deployed\], contract created at \[(?P<address>.*)\], fee left\[(?P<gas_left>\d+)\]')

    @staticmethod
    def parse(text):
        result = DeployedContract()
        for line in text.split('\n'):
            match = _PushContractParser._RE_PARSER.search(line)
            if not match:
                continue
            result.address = match.group(_PushContractParser._ADDRESS_TARGET)
            result.gas_left = int(match.group(_PushContractParser._GAS_LEFT_TARGET))

        if result:
            return result
        else:
            return None


class ContractResult:
    def __init__(self, gas_left=None, message=None):
        self.gas_left = gas_left
        self.message = message


class _MessageCallParser:
    _MESSAGE_TARGET = "message"
    _GAS_LEFT_TARGET = "gas_left"
    _RE_PARSER = re.compile(
        r'Remote call of smart contract call success -> \[Message call was successfully executed\], contract response\[(?P<message>.*)\], fee left\[(?P<gas_left>\d+)\]')

    @staticmethod
    def parse(text):
        result = ContractResult()
        for line in text.split('\n'):
            match = _MessageCallParser._RE_PARSER.search(line)
            if not match:
                continue
            result.message = match.group(_MessageCallParser._MESSAGE_TARGET)
            result.gas_left = int(match.group(_MessageCallParser._GAS_LEFT_TARGET))

        if result:
            return result
        else:
            return None


class _CompileContractParser:
    @staticmethod
    def parse(text):
        result = list()
        is_valid = False
        for line in text.split('\n'):
            if line == 'Compiled contracts:':
                is_valid = True
                continue
            if is_valid and line:
                result.append(line.strip())
        if result:
            return result
        else:
            return None


class _DecodeParser:
    @staticmethod
    def parse(text):
        return json.loads(text)


class _EncodeParser:
    @staticmethod
    def parse(text):
        return text[:len(text) - 1]


class _TransferParser:
    @staticmethod
    def parse(text):
        return text == "Transaction successfully performed\n"


class _GenerateKeysParser:
    _RE = {
        'keys_path': (re.compile(r'Generating key pair at "(?P<keys_path>.*)"'), str),
        'address': (re.compile(r'Address: (?P<address>.*)'), str),
    }

    @staticmethod
    def parse(text):
        result = dict()
        for line in text.split('\n'):
            for key, rx in _GenerateKeysParser._RE.items():
                match = rx[0].search(line)
                if match:
                    result[key] = _GenerateKeysParser._RE[key][1](
                        match.group(key))

        if len(result) != len(_GenerateKeysParser._RE):
            return None
        return result


class _KeysInfoParser:
    _TARGET = "address"
    _RE_PARSER = re.compile(r'Address: (?P<address>.*)')

    @staticmethod
    def parse(text):
        for line in text.split('\n'):
            match = _KeysInfoParser._RE_PARSER.search(line)
            if match:
                return match.group(_KeysInfoParser._TARGET)
        return None


MINIMUM_TX_WAIT = 1


class Node:
    DISTRIBUTOR_ADDRESS_PATH = os.path.realpath(
        os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "doc", "base-account-keys"))

    class Env:
        def __init__(self, binary_path):
            self.node_path = self.check(binary_path, "node")
            self.client_path = self.check(binary_path, "client")
            self.evm_path = self.check(binary_path, "libevmone.so.0.4")
            self.encoder_path = self.check(binary_path, "encoder.py")
            self.decoder_path = self.check(binary_path, "decoder.py")

        @staticmethod
        def check(binary_path, name):
            name_path = os.path.abspath(os.path.join(binary_path, name))
            if os.path.exists(name_path):
                return name_path
            else:
                raise Exception(f"file not found: {name_path}")

        def prepare(self, work_dir):
            if os.path.exists(work_dir):
                shutil.rmtree(work_dir, ignore_errors=True)
            os.makedirs(work_dir)

            shutil.copy(self.node_path, work_dir)
            shutil.copy(self.client_path, work_dir)
            shutil.copy(self.evm_path, work_dir)
            shutil.copy(self.encoder_path, work_dir)
            shutil.copy(self.decoder_path, work_dir)

    class Address:
        def __init__(self, key_path, address):
            self.key_path = key_path
            self.address = address

    class Id:
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

    class Settings:
        def __init__(self, node_id=None, start_up_time=2,
                     nodes=None, mining_thread=2,
                     database_path="likelib/database", clean_up_database=True):
            nodes = list() if nodes is None else nodes
            self.__settings_dict = {"id": node_id,
                                    "start_up_time": start_up_time,
                                    "nodes": nodes,
                                    "mining_thread": mining_thread,
                                    "database_path": database_path,
                                    "is_clean_up": clean_up_database}

        @property
        def id(self):
            return self.__settings_dict["id"]

        @id.setter
        def id(self, value):
            self.__settings_dict["id"] = value

        @property
        def start_up_time(self):
            return self.__settings_dict["start_up_time"]

        @start_up_time.setter
        def start_up_time(self, value):
            self.__settings_dict["start_up_time"] = value

        @property
        def nodes(self):
            return self.__settings_dict["nodes"]

        @nodes.setter
        def nodes(self, value):
            self.__settings_dict["nodes"] = value

        @property
        def mining_thread(self):
            return self.__settings_dict["mining_thread"]

        @mining_thread.setter
        def mining_thread(self, value):
            self.__settings_dict["mining_thread"] = value

        @property
        def database_path(self):
            return self.__settings_dict["database_path"]

        @database_path.setter
        def database_path(self, value):
            self.__settings_dict["database_path"] = value

        @property
        def is_clean_up(self):
            return self.__settings_dict["is_clean_up"]

        @is_clean_up.setter
        def is_clean_up(self, value):
            self.__settings_dict["is_clean_up"] = value

        def copy(self):
            new_settings = Node.Settings()
            new_settings.__settings_dict = self.__settings_dict.copy()

    def __init__(self, env, settings, logger):
        self.env = env
        self.settings = settings
        self.logger = logger

        self.name = "Node_" + str(self.settings.id.rpc_port)
        self.work_dir = os.path.abspath(self.name)

        self.logger.debug(f"{self.name} - work directory[{self.work_dir}]")

        env.prepare(self.work_dir)

        self.node_config_file = os.path.join(self.work_dir, "config.json")
        self.process = None
        self.__running = False

    @staticmethod
    def __generate_config(settings):
        config = {"net": {"listen_addr": settings.id.listen_sync_address,
                          "public_port": settings.id.sync_port},
                  "rpc": {"address": settings.id.listen_rpc_address},
                  "miner": {"threads": settings.mining_thread},
                  "nodes": [node_id.connect_sync_address for node_id in settings.nodes],
                  "keys_dir": ".",
                  "database": {"path": settings.database_path,
                               "clean": settings.is_clean_up}
                  }
        return json.dumps(config)

    def __run_standalone_command(self, *, command, parameters):
        run_commands = [self.env.client_path, command, *parameters]
        try:
            pipe = subprocess.run(run_commands, cwd=self.work_dir, capture_output=True)
        except Exception as e:
            raise Exception(f"exception at command execution {command} with parameters {parameters} : {e}")
        if pipe.returncode != 0:
            raise Exception(
                f"not success command execution {command} with parameters {parameters}: {pipe.stderr.decode('utf8')}")
        return pipe.stdout.decode("utf8")

    def __run_client_command(self, *, command, parameters, timeout, wait=0):
        run_commands = [self.env.client_path, command, "--host", self.settings.id.connect_rpc_address, *parameters]
        try:
            pipe = subprocess.run(run_commands, cwd=self.work_dir, capture_output=True, timeout=timeout)
            time.sleep(wait)
        except subprocess.TimeoutExpired as e:
            message = f"{self.name} - client slow command execution {command} with parameters {parameters} at node {self.settings.id.connect_rpc_address}"
            print(f"{self.name} - is in dead lock. pid {self.pid}, rpc address: {self.settings.id.connect_rpc_address}",
                  flush=True)
            self.logger.info(message)
            raise TimeOutException(message)
        except Exception as e:
            raise Exception(
                f"exception at command execution {command} with parameters {parameters} to node address {self.settings.id.connect_rpc_address} : {e}")
        if pipe.returncode != 0:
            raise Exception(
                f"not success command execution {command} with parameters {parameters} to node address {self.settings.id.connect_rpc_address}: {pipe.stderr.decode('utf8')}")
        return pipe.stdout.decode("utf8")

    def test(self, *, timeout=1):
        result = self.__run_client_command(command="test", parameters=[], timeout=timeout)
        return _TestParser.parse(result)

    def get_info(self, *, timeout=1):
        result = self.__run_client_command(command="info", parameters=[], timeout=timeout)
        return _InfoParser.parse(result)

    def get_block(self, *, block_hash, timeout=1):
        result = self.__run_client_command(command="get_block", parameters=["--hash", block_hash], timeout=timeout)
        return _BlockParser.parse(result)

    def get_balance(self, *, address, timeout=1):
        result = self.__run_client_command(command="get_balance", parameters=["--address", address.address],
                                           timeout=timeout)
        return _BalanceParser.parse(result)

    def run_check_test(self, timeout=1):
        TEST_CHECK(self.test(timeout=timeout), message=f"fail during connection test to node[{self.name}]")

    def run_check_balance(self, address, balance, timeout=1):
        TEST_CHECK_EQUAL(self.get_balance(address=address, timeout=timeout), balance,
                         message=f"fail during check balance test to node[{self.name}] by address: {address.address}")

    def run_check_transfer(self, to_address, amount, from_address, fee, timeout=1, wait=MINIMUM_TX_WAIT):
        TEST_CHECK(self.transfer(to_address=to_address, amount=amount, from_address=from_address,
                                 fee=fee, wait=wait, timeout=timeout),
                   message=f"fail during transfer test to node[{self.name}]")

    def generate_keys(self, *, keys_path):
        os.makedirs(keys_path)
        result = self.__run_standalone_command(command="generate", parameters=["--keys", keys_path])
        return _GenerateKeysParser.parse(result)

    def create_new_address(self, *, keys_path):
        keys_info = self.generate_keys(keys_path=os.path.join(self.work_dir, keys_path))
        return self.Address(keys_info['keys_path'], keys_info['address'])

    def get_keys_info(self, *, keys_path):
        result = self.__run_standalone_command(command="keys_info", parameters=["--keys", keys_path])
        return _KeysInfoParser.parse(result)

    def load_address(self, *, keys_path):
        address = self.get_keys_info(keys_path=keys_path)
        return self.Address(keys_path, address)

    def transfer(self, *, to_address, amount, from_address, fee, wait=MINIMUM_TX_WAIT, timeout=5):
        parameters = ["--to", to_address.address, "--amount",
                      str(amount), "--keys", from_address.key_path, "--fee", str(fee)]
        result = self.__run_client_command(
            command="transfer", parameters=parameters, timeout=timeout, wait=wait)
        return _TransferParser.parse(result)

    def compile_contract(self, *, code):
        result = self.__run_standalone_command(command="compile", parameters=["--code", code])
        return _CompileContractParser.parse(result)

    def encode_message(self, *, code, message):
        result = self.__run_standalone_command(command="encode", parameters=["--code", code, "--data", message])
        return _EncodeParser.parse(result)

    def decode_message(self, *, code, method, message):
        result = self.__run_standalone_command(command="decode",
                                               parameters=["--code", code, "--method", method, "--data",
                                                           message.message])
        return _DecodeParser.parse(result)

    def push_contract(self, *, from_address, code, fee, amount, init_message, timeout=1, wait=MINIMUM_TX_WAIT):
        parameters = ["--keys", from_address.key_path, "--code", code, "--amount",
                      str(amount), "--fee", str(fee), "--init", init_message]
        result = self.__run_client_command(
            command="create_contract", parameters=parameters, timeout=timeout, wait=wait)
        return _PushContractParser.parse(result)

    def message_to_contract(self, *, from_address, to_address, fee, amount, message, timeout=1, wait=MINIMUM_TX_WAIT):
        parameters = ["--keys", from_address.key_path, "--to", to_address.address,
                      "--amount", str(amount), "--fee", str(fee), "--message", message]
        result = self.__run_client_command(command="message_call", parameters=parameters, timeout=timeout, wait=wait)
        return _MessageCallParser.parse(result)

    def __write_config(self):
        node_config_content = Node.__generate_config(self.settings)
        with open(self.node_config_file, 'w') as node_config:
            node_config.write(node_config_content)
        self.logger.debug(
            f"{self.name} - config saved by path[{self.node_config_file}] with content[{node_config_content}]")

    def append_node_id(self, node_id):
        self.settings.nodes.append(node_id)

    @property
    def pid(self):
        if self.__running:
            return self.process.pid
        else:
            return -1

    def start_node(self, start_up_time=0):
        if self.__running:
            raise Exception(f"{self.name} - Process already started")

        self.__write_config()
        self.process = subprocess.Popen([self.env.node_path, "--config", self.node_config_file],
                                        cwd=self.work_dir, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)

        if self.process.poll() is None:
            self.logger.info(
                f"{self.name} - running node with work directory {self.work_dir}")
            self.__running = True
        else:
            self.logger.info(
                f"{self.name} - failed running node with work directory:{self.work_dir}")
            self.__running = False
            raise Exception(f"{self.name} - Process failed to start")

        self.logger.debug(
            f"{self.name} - start node(pid:{self.pid}) with work directory: {self.work_dir}")

        time.sleep(start_up_time)

    def __enter__(self):
        self.start_node(self.settings.start_up_time)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def close(self):
        if self.__running:
            pid = self.process.pid
            self.logger.info(
                f"{self.name} - try to close node with work_dir {self.work_dir}")
            self.process.send_signal(signal.SIGINT)
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.logger.info(
                    f"{self.name} - kill node with work_dir {self.work_dir}")
            exit_code = self.process.poll()
            self.__running = False
            self.logger.info(
                f"{self.name} - closed node(exit code:{exit_code}, pid:{pid}) with work_dir {self.work_dir}")


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
        return [node.settings.id for node in self.nodes]

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
    def create_pool_of_nodes_one_by_one(env, logger, start_sync_port, start_rpc_port, count_nodes):
        pool = NodePoll()
        pool.append(Node(env, Node.Settings(Node.Id(start_sync_port, start_rpc_port)), logger))

        for i in range(1, count_nodes):
            curent_sync_port = start_sync_port + i
            curent_rpc_port = start_rpc_port + i

            pool.append(
                Node(env, Node.Settings(Node.Id(curent_sync_port, curent_rpc_port), nodes=[pool.last.settings.id, ]),
                     logger))
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

        def test_case_runner(dependencies_folder):
            # change work directory for test
            if os.path.exists(__test_case_work_dir):
                shutil.rmtree(__test_case_work_dir, ignore_errors=True)
            os.makedirs(__test_case_work_dir)
            os.chdir(__test_case_work_dir)
            logger = Logger(test_name)

            def test_case_exception_wrapper(dependencies_folder):
                try:
                    env = Node.Env(dependencies_folder)
                    return func(env, logger)
                except Exception as error:
                    print(error, flush=True)
                    return 2

            return test_case_exception_wrapper(dependencies_folder)

        if test_name in __enabled_tests.keys() or test_name in __disabled_tests.keys():
            raise Exception(f"Test with this name[{test_name}] is exists")

        if disable:
            print(f"Registered test case [{test_name}] is disabled", flush=True)
            __disabled_tests[test_name] = test_case_runner
        else:
            print(f"Registered test case [{test_name}] is enabled", flush=True)
            __enabled_tests[test_name] = test_case_runner

        return test_case_runner

    return test_case_registrator


def run_registered_test_cases(pattern, dependencies_folder):
    success_tests = 0
    failed_tests = 0
    skipped_tests = 0

    try:
        matcher = re.compile(pattern)
    except Exception as e:
        matcher = None
        print(f"Invalid pattern: {e}", flush=True)
        exit(2)

    print(
        f"Enabled tests: {len(__enabled_tests)}. Disabled tests: {len(__disabled_tests)}.", flush=True)

    for registered_test_case_name in __enabled_tests:
        if matcher.match(registered_test_case_name) is None:
            skipped_tests += 1
            continue

        registered_test_case_runner = __enabled_tests[registered_test_case_name]

        print(f"Test case [{registered_test_case_name}] started.", flush=True)

        test_case_start_time = datetime.datetime.now()
        return_code = registered_test_case_runner(dependencies_folder)
        test_case_execute_time = datetime.datetime.now() - test_case_start_time

        if return_code == 0:
            print(
                f"Test case [{registered_test_case_name}] success. Execute time: {test_case_execute_time}.", flush=True)
            success_tests += 1
        else:
            print(
                f"Test case [{registered_test_case_name}] failed. Execute time: {test_case_execute_time}.", flush=True)
            failed_tests += 1

    all_tests = success_tests + failed_tests + skipped_tests
    print(
        f"All test cases: {all_tests}. Passed tests: {success_tests}. Failed tests: {failed_tests}. Skipped tests: {skipped_tests}.",
        flush=True)
    return failed_tests
