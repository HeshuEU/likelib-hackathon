import os
import time
import json
import signal
import subprocess

from .base import Logger, TimeOutException, InvalidArgumentsException
from .env import Env
from .parser import parser, Address, NodeInfo, TransferResult, Transaction


class Id:
    def __init__(self, sync_port, *, grpc_port=None, http_port=None, listening_addres="0.0.0.0",
                 absolute_address="127.0.0.1"):
        self.listening_addres = listening_addres
        self.absolute_address = absolute_address
        self.sync_port = sync_port
        self.grpc_port = grpc_port
        self.http_port = http_port

    @property
    def listen_sync_address(self) -> str:
        return f"{self.listening_addres}:{self.sync_port}"

    @property
    def connect_sync_address(self) -> str:
        return f"{self.absolute_address}:{self.sync_port}"

    @property
    def is_enable_grpc(self) -> bool:
        return self.grpc_port is not None

    @property
    def listen_grpc_address(self) -> str:
        if self.is_enable_grpc:
            return f"{self.listening_addres}:{self.grpc_port}"
        else:
            raise InvalidArgumentsException("grpc mode is off")

    @property
    def connect_grpc_address(self) -> str:
        if self.is_enable_grpc:
            return f"{self.absolute_address}:{self.grpc_port}"
        else:
            raise InvalidArgumentsException("grpc mode is off")

    @property
    def is_enable_http(self) -> bool:
        return self.http_port is not None

    @property
    def listen_http_address(self) -> str:
        if self.is_enable_http:
            return f"{self.listening_addres}:{self.http_port}"
        else:
            raise InvalidArgumentsException("http mode is off")

    @property
    def connect_http_address(self) -> str:
        if self.is_enable_http:
            return f"{self.absolute_address}:{self.http_port}"
        else:
            raise InvalidArgumentsException("http mode is off")


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
    def id(self) -> Id:
        return self.__settings_dict["id"]

    @id.setter
    def id(self, value: Id) -> None:
        self.__settings_dict["id"] = value

    @property
    def start_up_time(self) -> int:
        return self.__settings_dict["start_up_time"]

    @start_up_time.setter
    def start_up_time(self, value: int) -> None:
        self.__settings_dict["start_up_time"] = value

    @property
    def nodes(self) -> list:
        return self.__settings_dict["nodes"]

    @nodes.setter
    def nodes(self, value: list) -> None:
        self.__settings_dict["nodes"] = value

    @property
    def mining_thread(self) -> int:
        return self.__settings_dict["mining_thread"]

    @mining_thread.setter
    def mining_thread(self, value: int) -> None:
        self.__settings_dict["mining_thread"] = value

    @property
    def database_path(self) -> str:
        return self.__settings_dict["database_path"]

    @database_path.setter
    def database_path(self, value: str) -> None:
        self.__settings_dict["database_path"] = value

    @property
    def is_clean_up(self) -> bool:
        return self.__settings_dict["is_clean_up"]

    @is_clean_up.setter
    def is_clean_up(self, value: bool) -> None:
        self.__settings_dict["is_clean_up"] = value

    def copy(self):
        new_settings = Settings()
        new_settings.__settings_dict = self.__settings_dict.copy()
        return new_settings


ZERO_WAIT = 0
MINIMUM_TX_WAIT = 3
SIMPLE_PROCESS_TIMEOUT = 2
TRANSACTION_TIMEOUT = 5
NODE_START_WAIT = 2


class Node:
    def __init__(self, env: Env, settings: Settings, logger: Logger):
        self.env = env
        self.settings = settings
        self.logger = logger

        if self.settings.id.is_enable_grpc:
            self.name = "Node_grpc_" + str(self.settings.id.grpc_port)
        elif self.settings.id.is_enable_http:
            self.name = "Node_http_" + str(self.settings.id.http_port)
        else:
            self.name = "Node_sync_" + str(self.settings.id.sync_port)
        self.work_dir = os.path.abspath(self.name)

        self.logger.debug(f"{self.name} - work directory[{self.work_dir}]")

        self.env.prepare(self.work_dir)

        self.node_config_file = os.path.join(self.work_dir, "config.json")
        self.process = None
        self.is_running = False

    def __generate_config(self) -> str:
        config = {"net": {"listen_addr": self.settings.id.listen_sync_address,
                          "public_port": self.settings.id.sync_port},
                  "rpc": {},
                  "miner": {"threads": self.settings.mining_thread},
                  "nodes": [node_id.connect_sync_address for node_id in self.settings.nodes],
                  "keys_dir": ".",
                  "database": {"path": self.settings.database_path,
                               "clean": self.settings.is_clean_up}
                  }
        if self.settings.id.is_enable_grpc:
            config["rpc"]["grpc_address"] = self.settings.id.listen_grpc_address
        if self.settings.id.is_enable_http:
            config["rpc"]["http_address"] = self.settings.id.listen_http_address
        return json.dumps(config)

    def __write_config(self) -> None:
        node_config_content = self.__generate_config()
        with open(self.node_config_file, 'w') as node_config:
            node_config.write(node_config_content)
        self.logger.debug(
            f"{self.name} - config saved by path[{self.node_config_file}] with content[{node_config_content}]")

    def start_node(self, start_up_time=NODE_START_WAIT) -> None:
        if self.is_running:
            raise Exception(f"{self.name} - Process already started")

        self.__write_config()
        self.process = subprocess.Popen([self.env.node_path, "--config", self.node_config_file],
                                        cwd=self.work_dir, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)

        if self.process.poll() is None:
            self.logger.info(
                f"{self.name} - running node with work directory {self.work_dir}")
            self.is_running = True
        else:
            self.logger.info(
                f"{self.name} - failed running node with work directory:{self.work_dir}")
            self.is_running = False
            raise Exception(f"{self.name} - Process failed to start")

        self.logger.debug(
            f"{self.name} - start node(pid:{self.pid}) with work directory: {self.work_dir}")

        time.sleep(start_up_time)

    @property
    def pid(self) -> int:
        if self.is_running:
            return self.process.pid
        else:
            return -1

    def __enter__(self):
        self.start_node(self.settings.start_up_time)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def close(self):
        if self.is_running:
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
            self.is_running = False
            self.logger.info(
                f"{self.name} - closed node(exit code:{exit_code}, pid:{pid}) with work_dir {self.work_dir}")

    def __run_standalone_command(self, *, command, parameters, timeout):
        run_commands = [self.env.client_path, command, *parameters]
        try:
            pipe = subprocess.run(run_commands, cwd=self.work_dir, capture_output=True, timeout=timeout)
        except subprocess.TimeoutExpired as e:
            message = f"{self.name} - client slow command execution {command} with parameters {parameters}"
            self.logger.info(message)
            raise TimeOutException(message)
        except Exception as e:
            raise Exception(f"exception at command execution {command} with parameters {parameters} : {e}")
        if pipe.returncode != 0:
            raise Exception(
                f"not success command execution {command} with parameters {parameters}: {pipe.stderr.decode('utf8')}")
        return pipe.stdout.decode("utf8")

    def __run_client_command(self, *, command, parameters, timeout, is_http, wait=0):
        if is_http:
            run_commands = [self.env.client_path, command, "--host", self.settings.id.connect_http_address,
                            "--http", *parameters]
        else:
            run_commands = [self.env.client_path, command, "--host", self.settings.id.connect_grpc_address, *parameters]

        try:
            pipe = subprocess.run(run_commands, cwd=self.work_dir, capture_output=True, timeout=timeout)
            time.sleep(wait)
        except subprocess.TimeoutExpired as e:
            message = f"{self.name} - client slow command execution {command} with parameters {parameters}"
            print(f"{self.name} - is in dead lock. pid {self.pid}", flush=True)
            self.logger.info(message)
            raise TimeOutException(message)
        except Exception as e:
            raise Exception(f"exception at command execution {run_commands}: {e}")

        if pipe.returncode != 0:
            raise Exception(f"not success command execution {run_commands} : {pipe.stderr.decode('utf8')}")
        return pipe.stdout.decode("utf8")

    def connection_test(self, *, is_http=False, timeout=SIMPLE_PROCESS_TIMEOUT, wait=ZERO_WAIT) -> bool:
        result = self.__run_client_command(command="connection_test", parameters=[], timeout=timeout, is_http=is_http,
                                           wait=wait)
        return parser("connection_test").parse(result)

    def node_info(self, *, is_http=False, timeout=SIMPLE_PROCESS_TIMEOUT, wait=ZERO_WAIT) -> NodeInfo:
        result = self.__run_client_command(command="node_info", parameters=[], timeout=timeout, is_http=is_http,
                                           wait=wait)
        return parser("node_info").parse(result)

    def generate_keys(self, *, keys_path: str, timeout=SIMPLE_PROCESS_TIMEOUT) -> Address:
        path = os.path.abspath(keys_path)
        if not os.path.exists(path):
            os.makedirs(path)
        result = self.__run_standalone_command(command="generate_keys", parameters=["--keys", path], timeout=timeout)
        return parser("generate_keys").parse(result)

    def load_address(self, *, keys_path: str, timeout=SIMPLE_PROCESS_TIMEOUT) -> Address:
        result = self.__run_standalone_command(command="keys_info", parameters=["--keys", keys_path], timeout=timeout)
        return Address(keys_path, parser("keys_info").parse(result))

    def get_balance(self, *, address: Address, is_http=False, timeout=SIMPLE_PROCESS_TIMEOUT, wait=ZERO_WAIT) -> int:
        result = self.__run_client_command(command="get_balance", parameters=["--address", address.address],
                                           timeout=timeout, is_http=is_http, wait=wait)
        return parser("get_balance").parse(result)

    def transfer(self, *, to_address: Address, amount: int, from_address: Address, fee: int, is_http=False,
                 wait=ZERO_WAIT,
                 timeout=TRANSACTION_TIMEOUT) -> TransferResult:
        parameters = ["--to", to_address.address, "--amount", str(amount), "--keys", from_address.key_path, "--fee",
                      str(fee)]
        result = self.__run_client_command(command="transfer", parameters=parameters, timeout=timeout, is_http=is_http,
                                           wait=wait)
        return parser("transfer").parse(result)

    def get_transaction(self, *, tx_hash: str, is_http=False, wait=ZERO_WAIT,
                        timeout=TRANSACTION_TIMEOUT) -> Transaction:
        parameters = ["--hash", tx_hash]
        result = self.__run_client_command(command="get_transaction", parameters=parameters, timeout=timeout,
                                           is_http=is_http, wait=wait)
        return parser("get_transaction").parse(result)

    # def get_block(self, block_hash=None, block_number=None, *, is_http=False, timeout=SIMPLE_PROCESS_TIMEOUT,
    #               wait=ZERO_WAIT):
    #     if block_hash is not None:
    #         result = self.__run_client_command(command="get_block", parameters=["--hash", block_hash], timeout=timeout,
    #                                            is_http=is_http,
    #                                            wait=wait)
    #     elif block_number is not None:
    #         result = self.__run_client_command(command="get_block", parameters=["--number", block_number],
    #                                            timeout=timeout,
    #                                            is_http=is_http,
    #                                            wait=wait)
    #     else:
    #         raise InvalidArgumentsException("block_hash and block_number were not set")
    #     return parser("get_block").parse(result)

    # def compile_file(self, *, code: str) -> list:  # TODO
    #     result = self.__run_standalone_command(command="compile", parameters=["--code", code])
    #     return parser("compile").parse(result)
    #
    # def encode_message(self, *, code: str, message: str) -> str:  # TODO
    #     result = self.__run_standalone_command(command="encode", parameters=["--code", code, "--data", message])
    #     return parser("encode").parse(result)
    #
    # def decode_message(self, *, code: str, method: str, message: ContractResult) -> dict:  # TODO
    #     result = self.__run_standalone_command(command="decode",
    #                                            parameters=["--code", code, "--method", method, "--data",
    #                                                        message.message])
    #     return parser("decode").parse(result)
    #
    # def push_contract(self, *, from_address: Address, code: str, gas: int, amount: int, init_message: str,
    #                   timeout=TRANSACTION_TIMEOUT, wait=0) -> DeployedContract:  # TODO
    #     parameters = ["--keys", from_address.key_path, "--code", code, "--amount",
    #                   str(amount), "--gas", str(gas), "--init", init_message]
    #     result = self.__run_client_command(command="create_contract", parameters=parameters,
    #                                        timeout=timeout, wait=wait)
    #     return parser("create_contract").parse(result)
    #
    # def message_call(self, *, from_address: Address, to_address: DeployedContract, gas: int, amount: int, message: str,
    #                  timeout=TRANSACTION_TIMEOUT, wait=0) -> ContractResult:  # TODO
    #     parameters = ["--keys", from_address.key_path, "--to", to_address.address,
    #                   "--amount", str(amount), "--gas", str(gas), "--message", message]
    #     result = self.__run_client_command(command="message_call", parameters=parameters,
    #                                        timeout=timeout, wait=wait)
    #     return parser("message_call").parse(result)
