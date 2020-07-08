import json
import os
import shutil
import enum

from .base import Logger, InvalidArgumentsException, LogicException
from .node import Node
from .client import BaseClient
from .legacy_client import Client as LegacyClient
from .http_client import Client as HttpClient


class Id:
    def __init__(self, sync_port, *, grpc_port=None, http_port=None, listening_address="0.0.0.0",
                 absolute_address="127.0.0.1"):
        self.listening_address = listening_address
        self.absolute_address = absolute_address
        self.sync_port = sync_port
        self.grpc_port = grpc_port
        self.http_port = http_port

    @property
    def listen_sync_address(self) -> str:
        return f"{self.listening_address}:{self.sync_port}"

    @property
    def connect_sync_address(self) -> str:
        return f"{self.absolute_address}:{self.sync_port}"

    @property
    def is_enable_grpc(self) -> bool:
        return self.grpc_port is not None

    @property
    def listen_grpc_address(self) -> str:
        if self.is_enable_grpc:
            return f"{self.listening_address}:{self.grpc_port}"
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
            return f"{self.listening_address}:{self.http_port}"
        else:
            raise InvalidArgumentsException("http mode is off")

    @property
    def connect_http_address(self) -> str:
        if self.is_enable_http:
            return f"{self.absolute_address}:{self.http_port}"
        else:
            raise InvalidArgumentsException("http mode is off")


class NodeConfig:
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
        new_config = NodeConfig()
        new_config.__settings_dict = self.__settings_dict.copy()
        return new_config


NODE_STARTUP_TIME = 2


class ClientType(enum.Enum):
    LEGACY_GRPC = "legacy/grpc"
    LEGACY_HTTP = "legacy/http"
    PYTHON_HTTP = "python/http"


class Env:
    CLIENT_NAME = "client"
    NODE_NAME = "node"
    LIB_EVM_NAME = "libevmone.so.0.4"

    def __init__(self, *, binary_path: str, run_folder: str, test_name: str):
        self.binary_path = os.path.abspath(binary_path)
        Env.__check(self.binary_path, Env.CLIENT_NAME)
        Env.__check(self.binary_path, Env.NODE_NAME)
        Env.__check(self.binary_path, Env.LIB_EVM_NAME)

        self.dir = os.path.join(os.path.abspath(run_folder), test_name)
        Env.__prepare_directory(self.dir)
        os.chdir(self.dir)

        log_file_path = os.path.join(self.dir, "test.log")
        self.logger = Logger(test_name, log_file_path)
        self.__started_nodes = dict()
        self.__id_pool = dict()

    @staticmethod
    def __check(binary_path: str, name: str) -> str:
        file_path = os.path.join(binary_path, name)
        if os.path.exists(file_path):
            return file_path
        else:
            raise InvalidArgumentsException(f"file not found: {file_path}")

    @staticmethod
    def __prepare_directory(directory: str) -> None:
        if os.path.exists(directory):
            shutil.rmtree(directory, ignore_errors=True)
        os.makedirs(directory)

    @staticmethod
    def __generate_config(node_config: NodeConfig) -> str:
        config = {"net": {"listen_addr": node_config.id.listen_sync_address,
                          "public_port": node_config.id.sync_port,
                          "peers_db": "peers"},
                  "rpc": {},
                  "miner": {"threads": node_config.mining_thread},
                  "nodes": [node_id.connect_sync_address for node_id in node_config.nodes],
                  "keys_dir": ".",
                  "database": {"path": node_config.database_path,
                               "clean": node_config.is_clean_up}
                  }
        if node_config.id.is_enable_grpc:
            config["rpc"]["grpc_address"] = node_config.id.listen_grpc_address
        if node_config.id.is_enable_http:
            config["rpc"]["http_address"] = node_config.id.listen_http_address
        return json.dumps(config)

    def stop_node(self, node_id: Id):
        if self.is_node_started(node_id):
            associated_id = self.__find_node_id(node_id)
            node = self.__started_nodes[associated_id]
            del self.__started_nodes[associated_id]
            if self.__create_node_sync_id(associated_id) in self.__id_pool:
                del self.__id_pool[self.__create_node_sync_id(associated_id)]
            if associated_id.is_enable_grpc and self.__create_node_grpc_id(associated_id) in self.__id_pool:
                del self.__id_pool[self.__create_node_grpc_id(associated_id)]
            if associated_id.is_enable_http and self.__create_node_http_id(associated_id) in self.__id_pool:
                del self.__id_pool[self.__create_node_http_id(associated_id)]
            node.stop(shutdown_timeout=3)

    def close(self):
        for key in list(self.__started_nodes.keys()):
            self.stop_node(key)
        if self.__started_nodes or self.__id_pool:
            raise LogicException("control registers are not empty after cleaning")

    def __enter__(self):
        # Do nothing
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def __prepare_node_directory(self, node_directory: str) -> str:
        if not os.path.exists(node_directory):
            os.makedirs(node_directory)
        else:
            raise InvalidArgumentsException(f"node directory already exists:{node_directory}")

        shutil.copy(os.path.join(self.binary_path, Env.NODE_NAME), node_directory)
        shutil.copy(os.path.join(self.binary_path, Env.LIB_EVM_NAME), node_directory)
        return os.path.join(self.binary_path, Env.NODE_NAME)

    def __prepare_client_directory(self, client_directory: str) -> str:
        if not os.path.exists(client_directory):
            os.makedirs(client_directory)
        else:
            raise InvalidArgumentsException(f"client directory already exists:{client_directory}")

        shutil.copy(os.path.join(self.binary_path, Env.CLIENT_NAME), client_directory)
        return os.path.join(self.binary_path, Env.CLIENT_NAME)

    @staticmethod
    def __create_node_sync_id(node_id: Id):
        return "sync_" + str(node_id.sync_port)

    @staticmethod
    def __create_node_grpc_id(node_id: Id):
        return "grpc_" + str(node_id.sync_port)

    @staticmethod
    def __create_node_http_id(node_id: Id):
        return "http_" + str(node_id.sync_port)

    def __find_node_id(self, node_id: Id):
        if self.__create_node_sync_id(node_id) in self.__id_pool:
            return self.__id_pool[self.__create_node_sync_id(node_id)]
        if node_id.is_enable_grpc and self.__create_node_grpc_id(node_id) in self.__id_pool:
            return self.__id_pool[self.__create_node_grpc_id(node_id)]
        if node_id.is_enable_http and self.__create_node_http_id(node_id) in self.__id_pool:
            return self.__id_pool[self.__create_node_http_id(node_id)]
        return None

    def __append_node(self, node_id: Id, node: Node):
        self.__started_nodes[node_id] = node
        self.__id_pool[self.__create_node_sync_id(node_id)] = node_id
        if node_id.is_enable_grpc:
            self.__id_pool[self.__create_node_grpc_id(node_id)] = node_id
        if node_id.is_enable_http:
            self.__id_pool[self.__create_node_http_id(node_id)] = node_id

    def is_node_started(self, node_id: Id) -> bool:
        return self.__find_node_id(node_id) is not None

    def start_node(self, config: NodeConfig, startup_time=NODE_STARTUP_TIME) -> None:
        if self.is_node_started(config.id):
            raise InvalidArgumentsException(f"node already exists: {config.id}")
        for other_id in config.nodes:
            if not self.is_node_started(other_id):
                self.logger.debug(f"dependencies node was not started: {other_id}")

        node_name = "Node"
        if config.id.is_enable_grpc:
            node_name = node_name + "_grpc_" + str(config.id.grpc_port)
        if config.id.is_enable_http:
            node_name = node_name + "_http_" + str(config.id.http_port)
        node_name = node_name + "_sync_" + str(config.id.sync_port)

        node_work_dir = os.path.join(self.dir, node_name)
        node_file_path = self.__prepare_node_directory(node_work_dir)

        node_config_file_path = os.path.join(node_work_dir, "config.json")
        node_config_content = Env.__generate_config(config)
        with open(node_config_file_path, 'w') as node_config:
            node_config.write(node_config_content)

        node = Node(name=node_name, work_dir=node_work_dir, config_file_path=node_config_file_path,
                    node_file_path=node_file_path, logger=self.logger)

        node.start(startup_time=startup_time)
        self.__append_node(config.id, node)

    def __get_legacy_client(self, node_id: Id, is_http: bool) -> LegacyClient:
        client_name = "Client"
        if is_http:
            if not node_id.is_enable_http:
                raise InvalidArgumentsException("connection node doesn't support http protocol")
            client_name = client_name + "_to_node_http_" + str(node_id.http_port)
            connection_address = node_id.connect_http_address
        else:
            if not node_id.is_enable_grpc:
                raise InvalidArgumentsException("connection node doesn't support grpc protocol")
            client_name = client_name + "_to_node_grpc_" + str(node_id.grpc_port)
            connection_address = node_id.connect_grpc_address

        client_work_dir = os.path.join(self.dir, client_name)
        client_file_path = self.__prepare_client_directory(client_work_dir)
        return LegacyClient(name=client_name, client_file_path=client_file_path, work_dir=client_work_dir,
                            node_address=connection_address, is_http=is_http, logger=self.logger)

    def __get_http_client(self, node_id: Id) -> HttpClient:
        if not node_id.is_enable_http:
            raise InvalidArgumentsException("connection node doesn't support http protocol")
        client_name = "Client_python_to_node_http_" + str(node_id.http_port)

        client_work_dir = os.path.join(self.dir, client_name)
        if not os.path.exists(client_work_dir):
            os.makedirs(client_work_dir)
        else:
            raise InvalidArgumentsException(f"client directory already exists:{client_work_dir}")

        return HttpClient(name=client_name, work_dir=client_work_dir, connection_address=node_id.connect_http_address,
                          logger=self.logger)

    def get_client(self, client_type: ClientType, node_id: Id) -> BaseClient:
        if not self.is_node_started(node_id):
            raise InvalidArgumentsException(f"node does not exists: {node_id}")

        if not isinstance(client_type, ClientType):
            raise InvalidArgumentsException("not a client type")

        if client_type == ClientType.LEGACY_GRPC:
            return self.__get_legacy_client(node_id, False)
        elif client_type == ClientType.LEGACY_HTTP:
            return self.__get_legacy_client(node_id, True)
        elif client_type == ClientType.PYTHON_HTTP:
            return self.__get_http_client(node_id)
        else:
            raise LogicException()

    def get_grpc_client_to_outside_node(self, outside_node_id: Id) -> LegacyClient:
        connection_address = outside_node_id.connect_grpc_address
        client_name = "Client_to_node_" + connection_address.split(":")[0] + "_grpc"
        client_work_dir = os.path.join(self.dir, client_name)
        client_file_path = self.__prepare_client_directory(client_work_dir)
        return LegacyClient(name=client_name, client_file_path=client_file_path, work_dir=client_work_dir,
                            node_address=connection_address, is_http=False, logger=self.logger)


def get_distributor_address_path():
    return os.path.realpath(
        os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "doc", "base-account-keys"))
