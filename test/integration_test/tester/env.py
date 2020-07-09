import json
import os
import shutil

from .base import Logger, InvalidArgumentsException, LogicException
from .node import Node
from .websocket_client import Client


class Id:
    def __init__(self, sync_port, *, websocket_port=None, listening_address="0.0.0.0",
                 absolute_address="127.0.0.1"):
        self.listening_address = listening_address
        self.absolute_address = absolute_address
        self.sync_port = sync_port
        self.websocket_port = websocket_port

    @property
    def listen_sync_address(self) -> str:
        return f"{self.listening_address}:{self.sync_port}"

    @property
    def connect_sync_address(self) -> str:
        return f"{self.absolute_address}:{self.sync_port}"

    @property
    def listen_websocket_address(self) -> str:
        return f"{self.listening_address}:{self.websocket_port}"

    @property
    def connect_websocket_address(self) -> str:
        return f"{self.absolute_address}:{self.websocket_port}"


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


class Env:
    NODE_NAME = "node"
    LIB_EVM_NAME = "libevmone.so.0.4"

    def __init__(self, *, binary_path: str, run_folder: str, test_name: str):
        self.binary_path = os.path.abspath(binary_path)
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
                  "websocket": {"listening_addr": node_config.id.listen_websocket_address},
                  "miner": {"threads": node_config.mining_thread},
                  "nodes": [node_id.connect_sync_address for node_id in node_config.nodes],
                  "keys_dir": ".",
                  "database": {"path": node_config.database_path,
                               "clean": node_config.is_clean_up}
                  }
        return json.dumps(config)

    def stop_node(self, node_id: Id):
        if self.is_node_started(node_id):
            associated_id = self.__find_node_id(node_id)
            node = self.__started_nodes[associated_id]
            del self.__started_nodes[associated_id]
            if self.__create_node_sync_id(associated_id) in self.__id_pool:
                del self.__id_pool[self.__create_node_sync_id(associated_id)]
            if self.__create_node_ws_id(associated_id) in self.__id_pool:
                del self.__id_pool[self.__create_node_ws_id(associated_id)]
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

    @staticmethod
    def __create_node_sync_id(node_id: Id):
        return "sync_" + str(node_id.sync_port)

    @staticmethod
    def __create_node_ws_id(node_id: Id):
        return "ws_" + str(node_id.sync_port)

    def __find_node_id(self, node_id: Id):
        if self.__create_node_sync_id(node_id) in self.__id_pool:
            return self.__id_pool[self.__create_node_sync_id(node_id)]
        if self.__create_node_ws_id(node_id) in self.__id_pool:
            return self.__id_pool[self.__create_node_ws_id(node_id)]
        return None

    def __append_node(self, node_id: Id, node: Node):
        self.__started_nodes[node_id] = node
        self.__id_pool[self.__create_node_sync_id(node_id)] = node_id
        self.__id_pool[self.__create_node_ws_id(node_id)] = node_id

    def is_node_started(self, node_id: Id) -> bool:
        return self.__find_node_id(node_id) is not None

    def start_node(self, config: NodeConfig, startup_time=NODE_STARTUP_TIME) -> None:
        if self.is_node_started(config.id):
            raise InvalidArgumentsException(f"node already exists: {config.id}")
        for other_id in config.nodes:
            if not self.is_node_started(other_id):
                self.logger.debug(f"dependencies node was not started: {other_id}")

        node_name = "Node_ws_" + str(config.id.websocket_port) + "_sync_" + str(config.id.sync_port)

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

    def create_and_connect_client(self, node_id: Id) -> Client:
        if not self.is_node_started(node_id):
            raise InvalidArgumentsException(f"node does not exists: {node_id}")

        client_name = "Client_to_node_ws_" + str(node_id.websocket_port)

        client_work_dir = os.path.join(self.dir, client_name)
        if not os.path.exists(client_work_dir):
            os.makedirs(client_work_dir)
        else:
            raise InvalidArgumentsException(f"client directory already exists:{client_work_dir}")

        return Client(name=client_name, work_dir=client_work_dir, connection_address=node_id.connect_websocket_address,
                      logger=self.logger)


def get_distributor_address_path():
    return os.path.realpath(
        os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "doc", "base-account-keys"))
