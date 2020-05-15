import os
import time
import hashlib
import base64
import base58
import requests
import datetime
import coincurve

from .base import Logger, LogicException, TimeOutException, InvalidArgumentsException
from .client import NodeInfo, Keys, AccountInfo, TransferResult, Transaction, Block, DeployedContract, ContractResult, \
    TransactionStatus, BaseClient

ZERO_WAIT = 0
MINIMUM_TX_WAIT = 3
MINIMAL_CALL_TIMEOUT = 7
MINIMAL_STANDALONE_TIMEOUT = 5
MINIMAL_TRANSACTION_TIMEOUT = 10
MINIMAL_CONTRACT_TIMEOUT = 15


def _load_key(key_folder: str):
    path = os.path.abspath(key_folder)
    if not os.path.exists(path):
        InvalidArgumentsException(f"path not found {path}")

    key_path = os.path.join(path, "lkkey")
    try:
        with open(key_path, "rb") as file_out:
            private_bytes = file_out.read()
    except OSError as e:
        raise InvalidArgumentsException(e)
    return coincurve.PrivateKey.from_hex(private_bytes.decode())


def _save_key(key_folder: str, pk: coincurve.PrivateKey):
    path = os.path.abspath(key_folder)
    if not os.path.exists(path):
        os.makedirs(path)

    with open(os.path.join(path, "lkkey"), "wb") as file_out:
        file_out.write(pk.to_hex().encode())

    return path


def _key_to_address(key: coincurve.PrivateKey):
    pub_bytes = key.public_key.format(False)
    s = hashlib.new('sha256', pub_bytes).digest()
    r = hashlib.new('ripemd160', s).digest()
    return base58.b58encode(r).decode()


def _base64_to_hex(input_str: str) -> str:
    base64_bytes = input_str.encode()
    message_bytes = base64.b64decode(base64_bytes)
    return base58.b58encode(message_bytes).decode()


class _NodeInfoParser:
    @staticmethod
    def parse(result: dict) -> NodeInfo:
        return NodeInfo(_base64_to_hex(result["top_block_hash"]), result["top_block_number"])


class _GetBalanceParser:
    @staticmethod
    def parse(result: dict) -> int:
        return int(result["balance"])


class _TransferParser:
    @staticmethod
    def parse(result: dict, tx_hash: str) -> TransferResult:
        return TransferResult(tx_hash, result['status_code'] == 0, result['message'])


class _DeployedContract:
    @staticmethod
    def parse(result: dict, tx_hash: str) -> DeployedContract:
        if result['action_type'] == 3 and result['status_code'] == 0:
            return DeployedContract(tx_hash, result["message"], int(result["gas_left"]))


class _ContractCallParser:
    @staticmethod
    def parse(result: dict, tx_hash: str) -> ContractResult:
        return ContractResult(tx_hash, int(result["gas_left"]), result['message'])


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
        pk = coincurve.PrivateKey()
        path = _save_key(keys_path, pk)
        address = _key_to_address(pk)

        return Keys(path, address)

    def load_address(self, *, keys_path: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> Keys:
        pk = _load_key(keys_path)
        address = _key_to_address(pk)

        return Keys(keys_path, address)

    def get_balance(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> int:
        result = self.__run_client_command(route="/get_account", input_json={"address": address}, timeout=timeout,
                                           wait=wait)
        return _GetBalanceParser.parse(result)

    def get_account_info(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> AccountInfo:
        raise LogicException("method is not realized")

    def transfer(self, *, to_address: str, amount: int, from_address: Keys, fee: int, wait=MINIMUM_TX_WAIT,
                 timeout=MINIMAL_TRANSACTION_TIMEOUT) -> TransferResult:
        from_address_priv_key = _load_key(from_address.keys_path)
        from_address_address = _key_to_address(from_address_priv_key)
        timestamp = int(datetime.datetime.now().timestamp())
        tx = Transaction(Transaction.TRANSFER_TYPE, from_address_address, to_address, amount, fee, timestamp, "", True)
        tx_hash = bytes.fromhex(tx.hash())
        sign_bytes = from_address_priv_key.sign_recoverable(tx_hash)
        sign = base64.b64encode(sign_bytes)
        tx_json = {'from': from_address_address,
                   'to': to_address,
                   'amount': str(amount),
                   'fee': str(fee),
                   'timestamp': timestamp,
                   'data': {'message': ""},
                   'sign': sign}
        result = self.__run_client_command(route="/push_transaction", input_json=tx_json, timeout=timeout,
                                           wait=wait)
        return _TransferParser.parse(result, tx.hash())

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
        from_address_priv_key = _load_key(from_address.keys_path)
        from_address_address = _key_to_address(from_address_priv_key)
        timestamp = int(datetime.datetime.now().timestamp())
        to_address = base58.b58encode(bytes.fromhex('00' * 20)).decode()

        message_bytes = bytes.fromhex(init_message)
        message_len = len(message_bytes)
        message_len_bytes = message_len.to_bytes(8, byteorder='big')
        code_bytes = code.encode()
        code_len = len(code_bytes)
        code_len_bytes = code_len.to_bytes(8, byteorder='big')

        data = message_len_bytes + message_bytes + code_len_bytes + code_bytes

        tx = Transaction(Transaction.TRANSFER_TYPE, from_address_address, to_address, amount, fee, timestamp,
                         base64.b64encode(data).decode(), True)
        tx_hash = bytes.fromhex(tx.hash())
        sign_bytes = from_address_priv_key.sign_recoverable(tx_hash)
        sign = base64.b64encode(sign_bytes)
        encoded_message = base64.b64encode(message_bytes)
        tx_json = {'from': from_address_address,
                   'to': to_address,
                   'amount': str(amount),
                   'fee': str(fee),
                   'timestamp': timestamp,
                   'data': {'message': encoded_message, "abi": code},
                   'sign': sign}
        result = self.__run_client_command(route="/push_transaction", input_json=tx_json, timeout=timeout,
                                           wait=wait)
        return _DeployedContract.parse(result, tx.hash())

    def message_call(self, *, from_address: Keys, to_address: str, fee: int, amount: int, message: str,
                     timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> ContractResult:
        from_address_priv_key = _load_key(from_address.keys_path)
        from_address_address = _key_to_address(from_address_priv_key)
        timestamp = int(datetime.datetime.now().timestamp())
        data = base64.b64encode(bytes.fromhex(message)).decode()
        tx = Transaction(Transaction.TRANSFER_TYPE, from_address_address, to_address, amount, fee, timestamp, data,
                         True)
        tx_hash = bytes.fromhex(tx.hash())
        sign_bytes = from_address_priv_key.sign_recoverable(tx_hash)
        sign = base64.b64encode(sign_bytes)
        tx_json = {'from': from_address_address,
                   'to': to_address,
                   'amount': str(amount),
                   'fee': str(fee),
                   'timestamp': timestamp,
                   'data': {'message': data},
                   'sign': sign}
        result = self.__run_client_command(route="/push_transaction", input_json=tx_json, timeout=timeout,
                                           wait=wait)
        return _ContractCallParser.parse(result, tx.hash())

    def call_view(self, *, from_address: Keys, to_address: str, message: str, timeout=MINIMAL_CALL_TIMEOUT,
                  wait=ZERO_WAIT) -> str:
        from_address_priv_key = _load_key(from_address.keys_path)
        from_address_address = _key_to_address(from_address_priv_key)
        tx_json = {'from': from_address_address,
                   'to': to_address,
                   'message': base64.b64encode(bytes.fromhex(message)).decode()}
        result = self.__run_client_command(route="/call_contract_view", input_json=tx_json, timeout=timeout,
                                           wait=wait)
        return result
