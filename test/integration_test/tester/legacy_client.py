import hashlib
import os
import re
import subprocess
import time

from .base import Logger, InvalidArgumentsException, TimeOutException


class _TestConnectionParser:
    @staticmethod
    def parse(text: str) -> bool:
        return text == "Connection test passed\n"


class NodeInfo:
    def __init__(self, top_block_hash: str, top_block_number: int):
        self.top_block_hash = top_block_hash
        self.top_block_number = top_block_number


class _NodeInfoParser:
    _RE = {
        'top_block_hash': (re.compile(r'Top block hash: (?P<top_block_hash>.*)'), str),
        'top_block_number': (re.compile(r'Top block number: (?P<top_block_number>\d+)'), int),
    }

    @staticmethod
    def parse(text: str) -> NodeInfo:
        result = dict()
        for line in text.split('\n'):
            for key, rx in _NodeInfoParser._RE.items():
                match = rx[0].search(line)
                if match:
                    result[key] = _NodeInfoParser._RE[key][1](
                        match.group(key))

        if len(result) != len(_NodeInfoParser._RE):
            raise InvalidArgumentsException("Not full message")
        return NodeInfo(result["top_block_hash"], result["top_block_number"])


class Address:
    def __init__(self, key_path: str, address: str):
        self.key_path = key_path
        self.address = address


class _GenerateKeysParser:
    _RE = {
        'keys_path': (re.compile(r'Generating key pair at "(?P<keys_path>.*)"'), str),
        'address': (re.compile(r'Address: (?P<address>.*)'), str),
    }

    @staticmethod
    def parse(text: str) -> Address:
        result = dict()
        for line in text.split('\n'):
            for key, rx in _GenerateKeysParser._RE.items():
                match = rx[0].search(line)
                if match:
                    result[key] = _GenerateKeysParser._RE[key][1](
                        match.group(key))

        if len(result) != len(_GenerateKeysParser._RE):
            raise InvalidArgumentsException("Not full message")
        return Address(result["keys_path"], result["address"])


class _KeysInfoParser:
    _TARGET = "address"
    _RE_PARSER = re.compile(r'Address: (?P<address>.*)')

    @staticmethod
    def parse(text: str) -> str:
        for line in text.split('\n'):
            match = _KeysInfoParser._RE_PARSER.search(line)
            if match:
                return match.group(_KeysInfoParser._TARGET)
        raise InvalidArgumentsException("Not full message")


class _GetBalanceParser:
    _TARGET = "balance"
    _RE_PARSER = re.compile(r'Balance of (?P<address>.*) is (?P<balance>\d+)')

    @staticmethod
    def parse(text: str) -> int:
        match = _GetBalanceParser._RE_PARSER.search(text)
        if not match:
            raise InvalidArgumentsException("Not full message")
        return int(match.group(_GetBalanceParser._TARGET))


class TransferResult:
    def __init__(self, transaction_hash: str, result: bool, fail_message: str):
        self.transaction_hash = transaction_hash
        self.result = result
        self.fail_message = fail_message


class _TransferParser:
    _HASH_TARGET = "transaction_hash"
    _HASH_RE_PARSER = re.compile(r'Created transaction with hash\[hex\]: (?P<transaction_hash>.*) Transaction')
    _SUCCESS_MESSAGE = "Transaction successfully performed"
    _FAIL_TARGET = 'fail_message'
    _FAIL_RE_PARSER = re.compile(r'Transaction failed with message: (?P<fail_message>.*)')

    @staticmethod
    def parse(text: str) -> TransferResult:
        replaced_text = text.replace("\n", " ")
        match = _TransferParser._HASH_RE_PARSER.search(replaced_text)
        if match:
            tx_hash = match.group(_TransferParser._HASH_TARGET)
        else:
            raise InvalidArgumentsException("No tx hash result")

        if _TransferParser._SUCCESS_MESSAGE in replaced_text:
            return TransferResult(tx_hash, True, "")
        else:
            match = _TransferParser._FAIL_RE_PARSER.search(replaced_text)
            if match:
                fail_message = match.group(_TransferParser._HASH_TARGET)
                return TransferResult(tx_hash, False, fail_message)
            else:
                raise InvalidArgumentsException("No any fail message")


class Transaction:
    TRANSFER_TYPE = "transfer"
    CONTRACT_CREATION_TYPE = "contract_creation"
    CONTRACT_CALL_TYPE = "contract_call"

    def __init__(self, tx_type: str, from_address: str, to_address: str, value: int, fee: int, timestamp: int,
                 data: str, verified: bool):
        self.tx_type = tx_type
        self.from_address = from_address
        self.to_address = to_address
        self.value = value
        self.fee = fee
        self.timestamp = timestamp
        if data == "<empty>":
            self.data = ""
        else:
            self.data = data
        self.verified = verified

    def hash(self) -> str:
        string_for_hash = self.from_address + self.to_address + str(self.value) + str(self.fee) + str(
            self.timestamp) + self.data
        m = hashlib.sha256()
        m.update(string_for_hash.encode("utf8"))
        return m.hexdigest()


def _signature_checker(signature_status):
    return signature_status == "verified"


class _GetTransactionParser:
    _RE = {
        'tx_type': (re.compile(r'Type: (?P<tx_type>.*)'), str),
        'from_address': (re.compile(r'From: (?P<from_address>.*)'), str),
        'to_address': (re.compile(r'To: (?P<to_address>.*)'), str),
        'value': (re.compile(r'Value: (?P<value>\d+)'), int),
        'fee': (re.compile(r'Fee: (?P<fee>\d+)'), int),
        'timestamp': (re.compile(r'Timestamp: (?P<timestamp>\d+)'), int),
        'data': (re.compile(r'Data: (?P<data>.*)'), str),
        'signature': (re.compile(r'Signature: (?P<signature>.*)'), _signature_checker)
    }

    @staticmethod
    def parse(text) -> Transaction:
        result = dict()
        for line in text.split('\n'):
            for key, rx in _GetTransactionParser._RE.items():
                match = rx[0].search(line)
                if match:
                    result[key] = _GetTransactionParser._RE[key][1](
                        match.group(key))

        if len(result) != len(_GetTransactionParser._RE):
            raise InvalidArgumentsException("Not full message")
        return Transaction(result["tx_type"], result["from_address"], result["to_address"], result["value"],
                           result["fee"], result["timestamp"], result["data"], result["signature"])


#
# class Block:
#     def __init__(self, init_value):
#         self.init_value = init_value
#         for key in init_value.keys():
#             if type(init_value[key]) is dict:
#                 self.__dict__[key] = BlockObject(init_value[key])
#             elif type(init_value[key]) is list:
#                 self.__dict__[key] = list()
#                 for item in init_value[key]:
#                     self.__dict__[key].append(BlockObject(item))
#             else:
#                 self.__dict__[key] = init_value[key]
#
#     def serialize(self):
#         return self.init_value
#
#
# class _GetBlockParser:
#     _BLOCK_RE = {
#         'block_hash': (re.compile(r'Block hash (?P<block_hash>.*)'), str),
#         'depth': (re.compile(r'Depth: (?P<depth>\d+)'), int),
#         'timestamp': (re.compile(r'Timestamp: (?P<timestamp>\d+)'), int),
#         'coinbase': (re.compile(r'Coinbase: (?P<coinbase>.*)'), str),
#         'previous_block_hash': (re.compile(r'Previous block hash: (?P<previous_block_hash>.*)'), str),
#         'number_of_transactions': (re.compile(r'Number of transactions: (?P<number_of_transactions>\d+)'), int)
#     }
#
#     _TX_RE = {
#         'number': (re.compile(r'Transaction #(?P<number>\d+)'), int),
#         'type': (re.compile(r'Type: (?P<type>.*)'), str),
#         'from': (re.compile(r'From: (?P<from>.*)'), str),
#         'to': (re.compile(r'To: (?P<to>.*)'), str),
#         'value': (re.compile(r'Value: (?P<value>\d+)'), int),
#         'fee': (re.compile(r'Fee: (?P<fee>\d+)'), int),
#         'timestamp': (re.compile(r'Timestamp: (?P<timestamp>\d+)'), int),
#         'data': (re.compile(r'Data: (?P<data>.*)'), str),
#         'signature': (re.compile(r'Signature: (?P<signature>.*)'), _signature_checker)
#     }
#
#     @staticmethod
#     def _parse_line(rx_dict, line):
#         for key, rx in rx_dict.items():
#             match = rx[0].search(line)
#             if match:
#                 return key, match
#         return None, None
#
#     @staticmethod
#     def _add(result, key, match, target, target_type):
#         if key == target:
#             result[target] = target_type(match.group(target))
#
#     @staticmethod
#     def _parse_transaction(lines):
#         result = dict()
#         for line in lines:
#             key, match = _GetBlockParser._parse_line(_GetBlockParser._TX_RE, line)
#             for target in _GetBlockParser._TX_RE:
#                 _GetBlockParser._add(result, key, match, target,
#                                      _GetBlockParser._TX_RE[key][1])
#         if len(result) != len(_GetBlockParser._TX_RE):
#             return None
#         return result
#
#     @staticmethod
#     def parse(text):
#         result = dict()
#         lines = text.split('\n')
#         header_lines = lines[0:len(_GetBlockParser._BLOCK_RE)]
#         for line in header_lines:
#             key, match = _GetBlockParser._parse_line(_GetBlockParser._BLOCK_RE, line)
#             for target in _GetBlockParser._BLOCK_RE:
#                 _GetBlockParser._add(result, key, match, target,
#                                      _GetBlockParser._BLOCK_RE[key][1])
#         if len(result) != len(_GetBlockParser._BLOCK_RE):
#             return None
#
#         result["transactions"] = list()
#         for tx_number in range(result["number_of_transactions"]):
#             tx_lines = lines[len(_GetBlockParser._BLOCK_RE) + tx_number *
#                              len(_GetBlockParser._TX_RE):len(_GetBlockParser._BLOCK_RE) + (tx_number + 1) * len(
#                 _GetBlockParser._TX_RE)]
#             result["transactions"].append(
#                 _GetBlockParser._parse_transaction(tx_lines))
#
#         return result
#
#
# class DeployedContract:
#     def __init__(self, address=None, gas_left=None):
#         self.address = address
#         self.gas_left = gas_left
#
#
# class _PushContractParser:
#     _ADDRESS_TARGET = "address"
#     _GAS_LEFT_TARGET = "gas_left"
#     _RE_PARSER = re.compile(
#         r'Remote call of creation smart contract success -> \[Contract was successfully deployed\], contract created at \[(?P<address>.*)\], gas left\[(?P<gas_left>\d+)\]')
#
#     @staticmethod
#     def parse(text):
#         result = DeployedContract()
#         for line in text.split('\n'):
#             match = _PushContractParser._RE_PARSER.search(line)
#             if not match:
#                 continue
#             result.address = match.group(_PushContractParser._ADDRESS_TARGET)
#             result.gas_left = int(match.group(_PushContractParser._GAS_LEFT_TARGET))
#
#         if result:
#             return result
#         else:
#             return None
#
#
# class ContractResult:
#     def __init__(self, gas_left=None, message=None):
#         self.gas_left = gas_left
#         self.message = message
#
#
# class _MessageCallParser:
#     _MESSAGE_TARGET = "message"
#     _GAS_LEFT_TARGET = "gas_left"
#     _RE_PARSER = re.compile(
#         r'Remote call of smart contract call success -> \[Message call was successfully executed\], contract response\[(?P<message>.*)\], gas left\[(?P<gas_left>\d+)\]')
#
#     @staticmethod
#     def parse(text):
#         result = ContractResult()
#         for line in text.split('\n'):
#             match = _MessageCallParser._RE_PARSER.search(line)
#             if not match:
#                 continue
#             result.message = match.group(_MessageCallParser._MESSAGE_TARGET)
#             result.gas_left = int(match.group(_MessageCallParser._GAS_LEFT_TARGET))
#
#         if result:
#             return result
#         else:
#             return None
#
#
# class _TransferParser:
#     @staticmethod
#     def parse(text):
#         return text == "Transaction successfully performed\n"
#
#
# class _CompileContractParser:
#     @staticmethod
#     def parse(text):
#         result = list()
#         is_valid = False
#         for line in text.split('\n'):
#             if line == 'Compiled contracts:':
#                 is_valid = True
#                 continue
#             if is_valid and line:
#                 result.append(line.strip())
#         if result:
#             return result
#         else:
#             return None
#
#
# class _EncodeParser:
#     @staticmethod
#     def parse(text):
#         return text[:len(text) - 1]
#
#
# class _DecodeParser:
#     @staticmethod
#     def parse(text):
#         return json.loads(text)
#


ZERO_WAIT = 0
MINIMUM_TX_WAIT = 3
SIMPLE_PROCESS_TIMEOUT = 2
TRANSACTION_TIMEOUT = 5
NODE_START_WAIT = 2


class Client:
    def __init__(self, *, name: str, client_file_path: str, work_dir: str, node_address: str, is_http: bool,
                 logger: Logger):
        self.name = name
        self.client_file_path = client_file_path
        self.work_dir = work_dir
        self.connect_params = ["--host", node_address]
        if is_http:
            self.connect_params.append("--http")
        self.logger = logger

    def __run_standalone_command(self, *, command, parameters, timeout):
        run_commands = [self.client_file_path, command, *parameters]
        try:
            pipe = subprocess.run(run_commands, cwd=self.work_dir, capture_output=True, timeout=timeout)
        except subprocess.TimeoutExpired as e:
            message = f"{self.name} - client slow command execution {run_commands}"
            self.logger.info(message)
            raise TimeOutException(message)
        except Exception as e:
            raise Exception(f"exception at command execution {run_commands}: {e}")

        if pipe.returncode != 0:
            raise Exception(f"not success command execution {run_commands}: {pipe.stderr.decode('utf8')}")
        return pipe.stdout.decode("utf8")

    def __run_client_command(self, *, command, parameters, timeout, wait):
        run_commands = [self.client_file_path, command, *self.connect_params, *parameters]
        try:
            pipe = subprocess.run(run_commands, cwd=self.work_dir, capture_output=True, timeout=timeout)
        except subprocess.TimeoutExpired as e:
            message = f"{self.name} - client slow command execution {command} with parameters {parameters}"
            self.logger.info(message)
            raise TimeOutException(message)
        except Exception as e:
            raise Exception(f"exception at command execution {run_commands}: {e}")

        if pipe.returncode != 0:
            raise Exception(f"not success command execution {run_commands}: {pipe.stderr.decode('utf8')}")
        time.sleep(wait)
        return pipe.stdout.decode("utf8")

    def connection_test(self, *, timeout=SIMPLE_PROCESS_TIMEOUT, wait=ZERO_WAIT) -> bool:
        result = self.__run_client_command(command="connection_test", parameters=[], timeout=timeout, wait=wait)
        return _TestConnectionParser.parse(result)

    def node_info(self, *, timeout=SIMPLE_PROCESS_TIMEOUT, wait=ZERO_WAIT) -> NodeInfo:
        result = self.__run_client_command(command="node_info", parameters=[], timeout=timeout, wait=wait)
        return _NodeInfoParser.parse(result)

    def generate_keys(self, *, keys_path: str, timeout=SIMPLE_PROCESS_TIMEOUT) -> Address:
        path = os.path.abspath(keys_path)
        if not os.path.exists(path):
            os.makedirs(path)
        result = self.__run_standalone_command(command="generate_keys", parameters=["--keys", path], timeout=timeout)
        return _GenerateKeysParser.parse(result)

    def load_address(self, *, keys_path: str, timeout=SIMPLE_PROCESS_TIMEOUT) -> Address:
        result = self.__run_standalone_command(command="keys_info", parameters=["--keys", keys_path], timeout=timeout)
        return Address(keys_path, _KeysInfoParser.parse(result))

    def get_balance(self, *, address: Address, timeout=SIMPLE_PROCESS_TIMEOUT, wait=ZERO_WAIT) -> int:
        result = self.__run_client_command(command="get_balance", parameters=["--address", address.address],
                                           timeout=timeout, wait=wait)
        return _GetBalanceParser.parse(result)

    def transfer(self, *, to_address: Address, amount: int, from_address: Address, fee: int, wait=ZERO_WAIT,
                 timeout=TRANSACTION_TIMEOUT) -> TransferResult:
        parameters = ["--to", to_address.address, "--amount", str(amount), "--keys", from_address.key_path, "--fee",
                      str(fee)]
        result = self.__run_client_command(command="transfer", parameters=parameters, timeout=timeout, wait=wait)
        return _TransferParser.parse(result)

    def get_transaction(self, *, tx_hash: str, wait=ZERO_WAIT, timeout=TRANSACTION_TIMEOUT) -> Transaction:
        parameters = ["--hash", tx_hash]
        result = self.__run_client_command(command="get_transaction", parameters=parameters, timeout=timeout, wait=wait)
        return _GetTransactionParser.parse(result)

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
