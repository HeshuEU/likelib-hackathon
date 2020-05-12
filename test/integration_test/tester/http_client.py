import time
import base64
import requests
from Crypto.PublicKey import RSA

from .base import Logger, LogicException, TimeOutException
from .client import NodeInfo, Keys, AccountInfo, TransferResult, Transaction, Block, DeployedContract, ContractResult, TransactionStatus, BaseClient

ZERO_WAIT = 0
MINIMUM_TX_WAIT = 3
MINIMAL_CALL_TIMEOUT = 7
MINIMAL_STANDALONE_TIMEOUT = 5
MINIMAL_TRANSACTION_TIMEOUT = 10
MINIMAL_CONTRACT_TIMEOUT = 15


def base64_to_hex(input_str: str) -> str:
    base64_bytes = input_str.encode('ascii')
    message_bytes = base64.b64decode(base64_bytes)
    return message_bytes.hex()


class _NodeInfoParser:
    @staticmethod
    def parse(result: dict) -> NodeInfo:
        return NodeInfo(base64_to_hex(result["top_block_hash"]), result["top_block_number"])


class Client(BaseClient):
    def __init__(self, *, name: str, work_dir: str, connection_address: str, logger: Logger):
        self.name = name
        self.work_dir = work_dir
        self.connection_address = "http://" + connection_address
        self.logger = logger

    def __run_client_command(self, *, route: str, input_json: dict, timeout: int, wait: int):
        command = self.connection_address + route
        try:
            response = requests.post(command, json=input_json, timeout=timeout)
            if response.status_code != requests.codes.ok:
                raise Exception(f"not success command execution {command}[{response.status_code}]: {response.text}")
            result = response.json()
            if "status" in result:
                if result["status"] != "ok":
                    message = ""
                    if "result" in result:
                        message = result["result"]
                    raise Exception(f"exception at command execution to {command}: {message}")
            if "result" in result:
                time.sleep(wait)
                return result["result"]
            else:
                raise Exception(f"result message is not node message: {result}")
        except requests.exceptions.Timeout as e:
            message = f"client slow command execution {command} with parameters {input_json}"
            self.logger.info(f"{self.name} - " + message)
            raise TimeOutException(message)
        except ValueError as e:
            raise Exception(f"could not decode command response as json: {e}")
        except ConnectionError as e:
            raise Exception(f"could not connect to node with {command}: {e}")
        except Exception as e:
            raise Exception(f"exception at command execution to {command}: {e}")

    def connection_test(self, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> bool:
        result = self.__run_client_command(route="/get_node_info", input_json={}, timeout=timeout, wait=wait)
        return result is not None

    def node_info(self, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> NodeInfo:
        result = self.__run_client_command(route="/get_node_info", input_json={}, timeout=timeout, wait=wait)
        return _NodeInfoParser.parse(result)

    def generate_keys(self, *, keys_path: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> Keys:
        key = RSA.generate(2048)
        private_key = key.export_key()
        with open("private.pem", "wb") as file_out:
            file_out.write(private_key)

        public_key = key.publickey().export_key()
        with open("receiver.pem", "wb") as file_out:
            file_out.write(public_key)
        raise LogicException("method is not realized")

    def load_address(self, *, keys_path: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> Keys:
        raise LogicException("method is not realized")

    def get_balance(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> int:
        raise LogicException("method is not realized")

    def get_account_info(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> AccountInfo:
        raise LogicException("method is not realized")

    def transfer(self, *, to_address: str, amount: int, from_address: Keys, fee: int, wait=MINIMUM_TX_WAIT,
                 timeout=MINIMAL_TRANSACTION_TIMEOUT) -> TransferResult:
        raise LogicException("method is not realized")

    def get_transaction_status(self, *, tx_hash: str, wait: int, timeout: int) -> TransactionStatus:
        raise LogicException("method is not implemented")

    def get_transaction(self, *, tx_hash: str, wait=ZERO_WAIT, timeout=MINIMAL_CALL_TIMEOUT) -> Transaction:
        raise LogicException("method is not realized")

    def get_block(self, block_hash=None, block_number=None, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> Block:
        raise LogicException("method is not realized")

    def compile_file(self, *, code: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> list:
        raise LogicException("method is not realized")

    def encode_message(self, *, code: str, message: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> str:
        raise LogicException("method is not realized")

    def decode_message(self, *, code: str, method: str, message: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> dict:
        raise LogicException("method is not realized")

    def push_contract(self, *, from_address: Keys, code: str, fee: int, amount: int, init_message: str,
                      timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> DeployedContract:
        raise LogicException("method is not realized")

    def message_call(self, *, from_address: Keys, to_address: str, fee: int, amount: int, message: str,
                     timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> ContractResult:
        raise LogicException("method is not realized")

    def call_view(self, *, from_address: Keys, to_address: str, message: str, timeout=MINIMAL_CALL_TIMEOUT,
                  wait=ZERO_WAIT) -> str:
        raise LogicException("method is not realized")
