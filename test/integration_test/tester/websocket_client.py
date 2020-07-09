import os
import time
import hashlib
import base64
import base58
import datetime
import coincurve
import subprocess
import binascii
import json
import copy
import web3
import asyncio
import websockets
import threading

from .base import Logger, LogicException, TimeOutException, InvalidArgumentsException, BadResultException


class NodeInfo:
    def __init__(self, top_block_hash: str, top_block_number: int):
        self.top_block_hash = top_block_hash
        self.top_block_number = top_block_number


class Keys:
    def __init__(self, keys_path: str, address: str):
        self.keys_path = keys_path
        self.address = address


class AccountType(enum.Enum):
    CLIENT = "Client"
    CONTRACT = "Contract"


class AccountInfo:
    def __init__(self, account_type: AccountType, address: str, balance: int, nonce: int, tx_hashes: list):
        self.account_type = account_type
        self.address = address
        self.balance = balance
        self.nonce = nonce
        self.tx_hashes = tx_hashes


class TransactionStatusCode(enum.Enum):
    SUCCESS = "success"
    PENDING = "pending"
    BAD_QUERY_FORM = "bad_query_form"
    BAD_SIGN = "bad_sign"
    NOT_ENOUGH_BALANCE = "not_enough_balance"
    REVERT = "revert"
    FAILED = "failed"


class TransactionType(enum.Enum):
    TRANSFER = "transfer"
    CONTRACT_CREATION = "contract_creation"
    CONTRACT_CALL = "contract_call"
    NOT_CLASSIFIED = "can_not_be_classified"


class TransactionStatus:
    def __init__(self, action_type: TransactionType, status_code: TransactionStatusCode, tx_hash: str, fee: int,
                 data: str):
        self.action_type = action_type
        self.status_code = status_code
        self.tx_hash = tx_hash
        self.fee = fee
        self.data = data


class Transaction:
    def __init__(self, tx_type: TransactionType, from_address: str, to_address: str, value: int, fee: int,
                 timestamp: int, data: str, verified: bool):
        self.tx_type = tx_type
        self.from_address = from_address
        self.to_address = to_address
        self.value = value
        self.fee = fee
        self.timestamp = timestamp
        self.data = data
        self.verified = verified


class Block:
    def __init__(self, depth: int, nonce: int, timestamp: int, coinbase: str, previous_block_hash: str,
                 transactions: list):
        self.nonce = nonce
        self.depth = depth
        self.timestamp = timestamp
        self.coinbase = coinbase
        self.previous_block_hash = previous_block_hash
        self.transactions = transactions


def _base64_to_hex(input_str: str) -> str:
    base64_bytes = input_str.encode()
    message_bytes = base64.b64decode(base64_bytes)
    return base58.b58encode(message_bytes).decode()


def _generate_null_address() -> str:
    return base58.b58encode(bytes.fromhex('00' * 20)).decode()


class _TestConnectionParser:
    @staticmethod
    def parse(result: dict) -> bool:
        return result['api_version'] == 1


class _NodeInfoParser:
    @staticmethod
    def parse(result: dict) -> NodeInfo:
        return NodeInfo(_base64_to_hex(result["top_block_hash"]), result["top_block_number"])


class _GetBalanceParser:
    @staticmethod
    def parse(result: dict) -> int:
        return int(result["balance"])


class _GetAccountInfoParser:
    @staticmethod
    def parse(result: dict) -> AccountInfo:
        address = result["address"]
        balance = int(result["balance"])
        nonce = result["nonce"]
        account_type = AccountType(result['type'])
        txs_hashes = list()
        for item in result['transaction_hashes']:
            txs_hashes.append(_base64_to_hex(item))
        return AccountInfo(account_type, address, balance, nonce, txs_hashes)


class _TransactionStatusParser:
    @staticmethod
    def parse(result: dict, tx_hash: str) -> TransactionStatus:

        action_type_data = result["action_type"]
        if action_type_data == 0:
            action_type = TransactionType.NOT_CLASSIFIED
        elif action_type_data == 1:
            action_type = TransactionType.TRANSFER
        elif action_type_data == 2:
            action_type = TransactionType.CONTRACT_CALL
        elif action_type_data == 3:
            action_type = TransactionType.CONTRACT_CREATION
        else:
            raise InvalidArgumentsException("Invalid command output")

        status_code_data = result["status_code"]
        if status_code_data == 0:
            status_code = TransactionStatusCode.SUCCESS
        elif status_code_data == 1:
            status_code = TransactionStatusCode.PENDING
        elif status_code_data == 2:
            status_code = TransactionStatusCode.BAD_QUERY_FORM
        elif status_code_data == 3:
            status_code = TransactionStatusCode.BAD_SIGN
        elif status_code_data == 4:
            status_code = TransactionStatusCode.NOT_ENOUGH_BALANCE
        elif status_code_data == 5:
            status_code = TransactionStatusCode.REVERT
        elif status_code_data == 6:
            status_code = TransactionStatusCode.FAILED
        else:
            raise InvalidArgumentsException("Invalid command output")

        fee_left = int(result["fee_left"])

        if action_type == TransactionType.CONTRACT_CREATION:
            message = result["message"]
        else:
            message = base64.b64decode(result["message"]).hex()

        return TransactionStatus(action_type, status_code, tx_hash, fee_left, message)


class _GetTransactionParser:
    @staticmethod
    def parse(result: dict) -> Transaction:

        to_address = result["to"]
        from_address = result["from"]
        amount = int(result["amount"])
        fee = int(result["fee"])
        timestamp = result["timestamp"]
        tx_data = _base64_to_hex(result["data"])
        signature_status = False  # TODO create verification
        if to_address == _generate_null_address():
            tx = Transaction(TransactionType.CONTRACT_CREATION, from_address, to_address, amount, fee, timestamp,
                             tx_data, signature_status)
        elif tx_data == "":
            tx = Transaction(TransactionType.TRANSFER, from_address, to_address, amount, fee, timestamp, tx_data,
                             signature_status)
        else:
            tx = Transaction(TransactionType.CONTRACT_CALL, from_address, to_address, amount, fee, timestamp, tx_data,
                             signature_status)

        return tx


class _GetBlockParser:
    @staticmethod
    def parse(result: dict) -> Block:
        depth = result["depth"]
        nonce = result["nonce"]
        coinbase = result["coinbase"]
        previous_block_hash = _base64_to_hex(result["previous_block_hash"])
        timestamp = result["timestamp"]
        txs = list()
        for item in result['transactions']:
            txs.append(_GetTransactionParser.parse(item))
        return Block(depth, nonce, timestamp, coinbase, previous_block_hash, txs)


class _CallViewParser:
    @staticmethod
    def parse(result: dict) -> str:
        return base64.b64decode(result).hex()


ZERO_WAIT = 0
MINIMUM_TX_WAIT = 4
MINIMAL_CALL_TIMEOUT = 20
MINIMAL_STANDALONE_TIMEOUT = 10
MINIMAL_TRANSACTION_TIMEOUT = 20
MINIMAL_CONTRACT_TIMEOUT = 20
MAXIMUM_TRANSACTION_UPDATE_REQUEST=5


# необходимо два потока основной и принимающий
#


# async def hello():
#     uri = "ws://localhost:8765"
#     async with websockets.connect(uri) as websocket:
#         name = input("What's your name? ")
#
#         await websocket.send(name)
#         print(f"> {name}")
#
#         greeting = await websocket.recv()
#         print(f"< {greeting}")
#
#
# asyncio.get_event_loop().run_until_complete(hello())


class Client:
    KEY_FILE_NAME = "lkkey"
    CODE_FILE_NAME = "compiled_code.bin"
    METADATA_FILE_NAME = "metadata.json"
    COMPILER_NAME = 'solc'

    def __init__(self, *, name: str, work_dir: str, connection_address: str, logger: Logger):
        self.name = name
        self.work_dir = work_dir
        self.connection_address = "ws://" + connection_address
        self.logger = logger
        self.websocket = websockets.connect(self.connection_address)

    @staticmethod
    def __load_key(key_folder: str) -> coincurve.PrivateKey:
        path = os.path.abspath(key_folder)
        if not os.path.exists(path):
            InvalidArgumentsException(f"path not found {path}")

        key_path = os.path.join(path, Client.KEY_FILE_NAME)
        try:
            with open(key_path, "rb") as file_out:
                private_bytes = file_out.read()
        except OSError as e:
            raise InvalidArgumentsException(e)
        return coincurve.PrivateKey.from_hex(private_bytes.decode())

    @staticmethod
    def __save_key(key_folder: str, pk: coincurve.PrivateKey) -> str:
        path = os.path.abspath(key_folder)
        if not os.path.exists(path):
            os.makedirs(path)

        with open(os.path.join(path, Client.KEY_FILE_NAME), "wb") as file_out:
            file_out.write(pk.to_hex().encode())

        return path

    @staticmethod
    def __get_timestamp() -> int:
        return int(datetime.datetime.now().timestamp())

    @staticmethod
    def __calculate_transaction_hash(from_address: str, to_address: str, value: int, fee: int, timestamp: int,
                                     data: str) -> bytes:
        string_for_hash = from_address + to_address + str(value) + str(fee) + str(timestamp) + data
        m = hashlib.sha256()
        m.update(string_for_hash.encode())
        return m.digest()

    @staticmethod
    def __calculate_view_call_hash(from_address: str, to_address: str, timestamp: int, data: str) -> bytes:
        string_for_hash = from_address + to_address + str(timestamp) + data
        m = hashlib.sha256()
        m.update(string_for_hash.encode())
        return m.digest()

    @staticmethod
    def __key_to_address(key: coincurve.PrivateKey) -> str:
        pub_bytes = key.public_key.format(False)
        s = hashlib.new('sha256', pub_bytes).digest()
        r = hashlib.new('ripemd160', s).digest()
        return base58.b58encode(r).decode()

    def __run_client_command(self, *, route: str, input_json: dict, timeout: int, wait: int) -> dict:
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

    def __get_node_info(self, *, timeout, wait) -> dict:
        return self.__run_client_command(route="/get_node_info", input_json={}, timeout=timeout, wait=wait)

    def __get_account_info(self, *, address: str, timeout: int, wait: int) -> dict:
        return self.__run_client_command(route="/get_account", input_json={"address": address}, timeout=timeout,
                                         wait=wait)

    def __get_transaction(self, *, tx_hash: str, timeout: int, wait: int) -> dict:
        return self.__run_client_command(route="/get_transaction", input_json={"hash": tx_hash}, timeout=timeout,
                                         wait=wait)

    def __get_transaction_status(self, *, tx_hash: str, timeout: int, wait: int) -> dict:
        return self.__run_client_command(route="/get_transaction_status", input_json={"hash": tx_hash}, timeout=timeout,
                                         wait=wait)

    def __get_block(self, *, block_hash=None, block_number=None, timeout: int, wait: int) -> dict:
        if block_hash is not None:
            input_json = {"hash": base64.b64encode(bytes.fromhex(block_hash))}
        elif block_number is not None:
            input_json = {"number": block_number}
        else:
            raise InvalidArgumentsException("No block_hash and block_number")
        return self.__run_client_command(route="/get_block", input_json=input_json, timeout=timeout, wait=wait)

    def __push_transaction(self, *, from_address: str, to_address: str, amount: int, fee: int, timestamp: int,
                           message: str, sign: str, timeout: int, wait: int) -> dict:
        tx_json = {'from': from_address,
                   'to': to_address,
                   'amount': str(amount),
                   'fee': str(fee),
                   'timestamp': timestamp,
                   'data': message,
                   'sign': sign}
        return self.__run_client_command(route="/push_transaction", input_json=tx_json, timeout=timeout,
                                         wait=wait)

    def __call_contract_view(self, *, from_address: str, to_address: str, timestamp: int, message: str, sign: str,
                             timeout: int, wait: int) -> dict:
        tx_json = {'from': from_address,
                   'to': to_address,
                   'timestamp': timestamp,
                   'message': message,
                   'sign': sign}
        return self.__run_client_command(route="/call_contract_view", input_json=tx_json, timeout=timeout,
                                         wait=wait)

    def connection_test(self, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> bool:
        result = self.__get_node_info(timeout=timeout, wait=wait)
        return _TestConnectionParser.parse(result)

    def node_info(self, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> NodeInfo:
        result = self.__get_node_info(timeout=timeout, wait=wait)
        return _NodeInfoParser.parse(result)

    def generate_keys(self, *, keys_path: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> Keys:
        pk = coincurve.PrivateKey()
        path = Client.__save_key(keys_path, pk)
        address = Client.__key_to_address(pk)
        return Keys(path, address)

    def load_address(self, *, keys_path: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> Keys:
        pk = Client.__load_key(keys_path)
        address = Client.__key_to_address(pk)
        return Keys(keys_path, address)

    def get_balance(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> int:
        result = self.__get_account_info(address=address, timeout=timeout, wait=wait)
        return _GetBalanceParser.parse(result)

    def get_account_info(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> AccountInfo:
        result = self.__get_account_info(address=address, timeout=timeout, wait=wait)
        return _GetAccountInfoParser.parse(result)

    def transfer(self, *, to_address: str, amount: int, from_address: Keys, fee: int, wait=MINIMUM_TX_WAIT,
                 timeout=MINIMAL_TRANSACTION_TIMEOUT) -> TransactionStatus:
        from_address_private_key = Client.__load_key(from_address.keys_path)
        from_address = Client.__key_to_address(from_address_private_key)
        timestamp = self.__get_timestamp()
        tx_hash = Client.__calculate_transaction_hash(from_address, to_address, amount, fee, timestamp, "")
        sign = base64.b64encode(from_address_private_key.sign_recoverable(tx_hash)).decode()
        result = self.__push_transaction(from_address=from_address, to_address=to_address, amount=amount, fee=fee,
                                         timestamp=timestamp, message="", sign=sign, timeout=timeout, wait=wait)
        return _TransactionStatusParser.parse(result, tx_hash.hex())

    def get_transaction_status(self, *, tx_hash: str, wait=ZERO_WAIT,
                               timeout=MINIMAL_CALL_TIMEOUT) -> TransactionStatus:
        result = self.__get_transaction_status(tx_hash=base64.b64encode(bytes.fromhex(tx_hash)).decode(),
                                               timeout=timeout, wait=wait)
        return _TransactionStatusParser.parse(result, tx_hash)

    def get_transaction(self, *, tx_hash: str, wait=ZERO_WAIT, timeout=MINIMAL_CALL_TIMEOUT) -> Transaction:
        result = self.__get_transaction(tx_hash=base64.b64encode(bytes.fromhex(tx_hash)).decode(), wait=wait,
                                        timeout=timeout)
        return _GetTransactionParser.parse(result)

    def get_block(self, block_hash=None, block_number=None, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> Block:
        result = self.__get_block(block_hash=block_hash, block_number=block_number, timeout=timeout, wait=wait)
        return _GetBlockParser.parse(result)

    @staticmethod
    def __parse_solidity_output(output) -> dict:
        output_lines = output.split('\n')
        group_number = 4
        result = dict()
        for i in range(int(len(output_lines) / group_number)):
            starter_index = i * group_number
            header = output_lines[starter_index + 1]
            data = output_lines[starter_index + 3]
            contract_name = header.split(" ")[1].split(':')[1]
            result[contract_name] = data
        return result

    def __compile_solidity_file(self, solidity_file_path: str, timeout: int):
        try:
            pipe = subprocess.run([Client.COMPILER_NAME, "--bin", solidity_file_path], cwd=self.work_dir,
                                  capture_output=True, timeout=timeout)
            if pipe.returncode != 0:
                raise BadResultException(
                    f"failed at compilation of file success compilation {solidity_file_path}: {pipe.stderr.decode('utf8')}")
            binary_output = pipe.stdout.decode('utf8')
        except subprocess.TimeoutExpired:
            message = f"{self.name} - slow compilation of file {solidity_file_path}"
            self.logger.info(message)
            raise TimeOutException(message)

        return self.__parse_solidity_output(binary_output)

    def __create_metadata_of_solidity_file(self, solidity_file_path: str, timeout: int):
        try:
            pipe = subprocess.run([Client.COMPILER_NAME, "--metadata", solidity_file_path], cwd=self.work_dir,
                                  capture_output=True, timeout=timeout)
            if pipe.returncode != 0:
                raise BadResultException(
                    f"not success abi generation {solidity_file_path}: {pipe.stderr.decode('utf8')}")
            abi_output = pipe.stdout.decode('utf8')
        except subprocess.TimeoutExpired as e:
            message = f"{self.name} - slow abi generation by file {solidity_file_path}"
            self.logger.info(message)
            raise TimeOutException(message)

        return self.__parse_solidity_output(abi_output)

    def compile_file(self, *, code: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> list:
        solidity_file_path = os.path.abspath(code)
        binary_result = self.__compile_solidity_file(solidity_file_path, timeout)

        for contract_name in binary_result.keys():
            contract_folder = os.path.join(self.work_dir, contract_name)
            if not os.path.exists(contract_folder):
                os.makedirs(contract_folder)

            with open(os.path.join(contract_folder, Client.CODE_FILE_NAME), 'wt', encoding='ascii') as f:
                f.write(binary_result[contract_name])

        abi_result = self.__create_metadata_of_solidity_file(solidity_file_path, timeout)
        result = list()
        for contract_name in abi_result.keys():
            contract_folder = os.path.join(self.work_dir, contract_name)
            result.append(contract_folder)

            serialized_abi = json.dumps(json.loads(abi_result[contract_name]), separators=(',', ': '), indent=4)
            with open(os.path.join(contract_folder, Client.METADATA_FILE_NAME), 'wt', encoding='ascii') as f:
                f.write(serialized_abi)
        return result

    @staticmethod
    def __create_function_hash(function):
        name = function.abi['name']
        str_view = str(function)
        signature = str_view[str_view.find(" ") + 1: len(str_view) - 1]
        sha3_hash = web3.Web3.solidityKeccak(['bytes'], [signature.encode('ascii')]).hex()
        return name, signature, sha3_hash[2:10]

    @staticmethod
    def __encode_call(compiled_sol, call):
        new_contract_data_abi = copy.deepcopy(compiled_sol['metadata']['output']['abi'])
        for item in new_contract_data_abi:
            for input_item in item["inputs"]:
                if input_item["type"] == "address":
                    input_item["internalType"] = "bytes32"
                    input_item["type"] = "bytes32"

        new_contract = web3.Web3().eth.contract(abi=new_contract_data_abi, bytecode=compiled_sol['bytecode'])

        if call['method'] == "constructor":
            call_data = new_contract.constructor(*call["args"]).data_in_transaction
            call_data = call_data[2:]
        else:
            call_data = new_contract.encodeABI(fn_name=call["method"], args=call["args"])

            original = web3.Web3().eth.contract(abi=compiled_sol['metadata']['output']['abi'])
            target_hash = Client.__create_function_hash(original.get_function_by_name(call["method"]))
            call_data = target_hash[2] + call_data[10:]

        return call_data

    @staticmethod
    def __load_contract_data(path_to_contract_folder):
        compiled_contract_file_path = os.path.join(path_to_contract_folder, Client.CODE_FILE_NAME)
        if not os.path.exists(compiled_contract_file_path):
            raise InvalidArgumentsException(f"contract file not exists by path: {compiled_contract_file_path}")
        with open(compiled_contract_file_path, "rt") as f:
            bytecode = f.read()

        contract_metadata_file_path = os.path.join(path_to_contract_folder, Client.METADATA_FILE_NAME)
        if not os.path.exists(contract_metadata_file_path):
            raise InvalidArgumentsException(f"contract metadata file not exists by path: {contract_metadata_file_path}")
        with open(contract_metadata_file_path, "rt") as f:
            metadata = json.loads(f.read())

        return {"bytecode": bytecode, "metadata": metadata}

    @staticmethod
    def __parse_call(call_string):
        ADDRESS_FN = 'Address('
        bracket_index = call_string.find('(')
        method_name = call_string[0: bracket_index]
        call_string = call_string[bracket_index + 1: len(call_string) - 1]
        while call_string.find(ADDRESS_FN) != -1:
            fn_start = call_string.find(ADDRESS_FN)
            fn_end = fn_start + len(ADDRESS_FN)
            end = call_string.find(')', fn_end)
            left_part = call_string[0: fn_start]
            right_part = call_string[end + 1:]
            input_data = call_string[fn_end: end]
            converted_data = base58.b58decode(input_data)
            converted_address = f'"{(bytes(32 - len(converted_data)) + converted_data).hex()}"'
            call_string = left_part + converted_address + right_part
        argument_data = f"[{call_string}]"
        arguments = json.loads(argument_data)
        return {"method": method_name, "args": arguments}

    def encode_message(self, *, code: str, message: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> str:
        parsed_call = Client.__parse_call(message)
        contract_data = Client.__load_contract_data(code)
        call_data = Client.__encode_call(contract_data, parsed_call)
        if not call_data:
            raise BadResultException("dad encoding")
        return call_data

    @staticmethod
    def __decode_output(compiled_sol, method, data):
        web3_interface = web3.Web3()

        for abi_fn in compiled_sol['metadata']['output']['abi']:
            if abi_fn["type"] == "function":
                if abi_fn["name"] == method:
                    remake_abi = abi_fn
                    remake_abi['inputs'] = remake_abi['outputs']
                    remake_abi['outputs'] = ""
                    remake_contract = web3_interface.eth.contract(abi=[remake_abi])
                    target_hash = Client.__create_function_hash(remake_contract.get_function_by_name(method))
                    return remake_contract.decode_function_input(target_hash[2] + data)[1]
        return None

    @staticmethod
    def __prepare_for_serialize(decoded_data):
        if type(decoded_data) is list:
            for item_num in range(len(decoded_data)):
                decoded_data[item_num] = Client.__prepare_for_serialize(decoded_data[item_num])
        elif type(decoded_data) is bytes:
            b = binascii.hexlify(decoded_data)
            return b.decode('utf8')
        elif type(decoded_data) is dict:
            for item_key in decoded_data.keys():
                decoded_data[item_key] = Client.__prepare_for_serialize(decoded_data[item_key])
        return decoded_data

    def decode_message(self, *, code: str, message: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> dict:
        contract_data = Client.__load_contract_data(code)
        decoded_data = Client.__decode_output(contract_data, message)
        if not decoded_data:
            raise BadResultException("dad encoding")
        return Client.__prepare_for_serialize(decoded_data)

    def push_contract(self, *, from_address: Keys, code: str, fee: int, amount: int, init_message: str,
                      timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> TransactionStatus:

        from_address_private_key = Client.__load_key(from_address.keys_path)
        from_address = Client.__key_to_address(from_address_private_key)
        to_address = _generate_null_address()
        timestamp = self.__get_timestamp()
        message_bytes = bytes.fromhex(init_message)
        tx_hash = Client.__calculate_transaction_hash(from_address, to_address, amount, fee, timestamp,
                                                      base64.b64encode(message_bytes).decode())
        sign = base64.b64encode(from_address_private_key.sign_recoverable(tx_hash)).decode()
        result = self.__push_transaction(from_address=from_address, to_address=to_address, amount=amount, fee=fee,
                                         timestamp=timestamp, message=base64.b64encode(message_bytes).decode(),
                                         sign=sign, timeout=timeout, wait=wait)
        return _TransactionStatusParser.parse(result, tx_hash.hex())

    def message_call(self, *, from_address: Keys, to_address: str, fee: int, amount: int, message: str,
                     timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> TransactionStatus:
        from_address_private_key = Client.__load_key(from_address.keys_path)
        from_address = Client.__key_to_address(from_address_private_key)
        timestamp = self.__get_timestamp()
        data = base64.b64encode(bytes.fromhex(message)).decode()
        tx_hash = Client.__calculate_transaction_hash(from_address, to_address, amount, fee, timestamp, data)
        sign = base64.b64encode(from_address_private_key.sign_recoverable(tx_hash)).decode()
        result = self.__push_transaction(from_address=from_address, to_address=to_address, amount=amount, fee=fee,
                                         timestamp=timestamp, message=data, sign=sign, timeout=timeout, wait=wait)
        return _TransactionStatusParser.parse(result, tx_hash.hex())

    def call_view(self, *, from_address: Keys, to_address: str, message: str, timeout=MINIMAL_CALL_TIMEOUT,
                  wait=ZERO_WAIT) -> str:
        from_address_private_key = Client.__load_key(from_address.keys_path)
        from_address = Client.__key_to_address(from_address_private_key)
        data = base64.b64encode(bytes.fromhex(message)).decode()
        timestamp = self.__get_timestamp()
        call_hash = self.__calculate_view_call_hash(from_address, to_address, timestamp, data)
        sign = base64.b64encode(from_address_private_key.sign_recoverable(call_hash)).decode()
        result = self.__call_contract_view(from_address=from_address, to_address=to_address, timestamp=timestamp,
                                           message=data, sign=sign, timeout=timeout, wait=wait)
        return _CallViewParser.parse(result)

    def transaction_success_wait(self, *, transaction: TransactionStatus, update_time=MINIMUM_TX_WAIT,
                                 max_request=MAXIMUM_TRANSACTION_UPDATE_REQUEST) -> bool:
        stat = self.get_transaction_status(tx_hash=transaction.tx_hash)
        self.logger.info(f"Wait transaction {transaction.tx_hash} (transaction_update_time = {update_time}, max_update_request = {max_request})")
        request_count = 0
        while stat.status_code != TransactionStatusCode.SUCCESS:
            time.sleep(update_time)
            stat = self.get_transaction_status(tx_hash=transaction.tx_hash)
            request_count += 1
            self.logger.info(f"Wait transaction {transaction.tx_hash} request_count = {request_count}")
            if request_count >= max_request: return False
        self.logger.info(f"Transaction {transaction.tx_hash} success.")
        return True
