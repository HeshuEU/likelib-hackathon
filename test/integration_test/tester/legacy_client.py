import json
import os
import re
import subprocess
import time

from .base import Logger, InvalidArgumentsException, TimeOutException, LogicException
from .client import NodeInfo, Keys, AccountInfo, TransferResult, Transaction, Block, DeployedContract, ContractResult, \
    TransactionStatus, BaseClient


class _TestConnectionParser:
    @staticmethod
    def parse(text: str) -> bool:
        return text == "Connection test passed\n"


class _NodeInfoParser:
    _RE = {
        'top_block_hash': (re.compile(r'Top block hash: (?P<top_block_hash>.*)'), str),
        'top_block_number': (re.compile(r'Top block number: (?P<top_block_number>\d+)'), int),
    }

    @staticmethod
    def parse(text: str) -> NodeInfo:
        result = dict()
        for key, rx in _NodeInfoParser._RE.items():
            match = rx[0].search(text)
            if match:
                result[key] = _NodeInfoParser._RE[key][1](match.group(key))

        if len(result) != len(_NodeInfoParser._RE):
            raise InvalidArgumentsException("Not full message")
        return NodeInfo(result["top_block_hash"], result["top_block_number"])


class _GenerateKeysParser:
    _RE = {
        'keys_path': (re.compile(r'Generating key pair at "(?P<keys_path>.*)"'), str),
        'address': (re.compile(r'Address: (?P<address>.*)'), str),
    }

    @staticmethod
    def parse(text: str) -> Keys:
        result = dict()
        for line in text.split('\n'):
            for key, rx in _GenerateKeysParser._RE.items():
                match = rx[0].search(line)
                if match:
                    result[key] = _GenerateKeysParser._RE[key][1](match.group(key))

        if len(result) != len(_GenerateKeysParser._RE):
            raise InvalidArgumentsException("Not full message")
        return Keys(result["keys_path"], result["address"])


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


class _GetAccountInfoParser:
    _BALANCE_TARGET = "balance"
    _BALANCE_RE_PARSER = re.compile(r'Balance: (?P<balance>\d+)')
    _ADDRESS_TARGET = "address"
    _ADDRESS_RE_PARSER = re.compile(r'address: (?P<address>.*)')
    _NONCE_TARGET = "nonce"
    _NONCE_RE_PARSER = re.compile(r'Nonce: (?P<nonce>\d+)')
    _TYPE_TARGET = "type"
    _TYPE_RE_PARSER = re.compile(r'(?P<type>.*) address: ')
    _ABI_TARGET = "abi"
    _ABI_RE_PARSER = re.compile(r'ABI: \n(?P<abi>.*)')  # TODO

    @staticmethod
    def parse(text: str) -> AccountInfo:
        print(text)
        result = dict()
        lines = text.split('\n')
        header_lines = lines[0:len(_GetAccountInfoParser._RE)]
        for line in header_lines:
            for key, rx in _GetAccountInfoParser._RE.items():
                match = rx[0].search(line)
                if match:
                    result[key] = _GetAccountInfoParser._RE[key][1](match.group(key))

        if len(result) != len(_GetAccountInfoParser._RE):
            raise InvalidArgumentsException("Not full message")

        number_of_hashes = len(lines) - len(_GetAccountInfoParser._RE) - 1
        result["tx_hashes"] = list()
        tx_hash_re = re.compile(r'(?P<tx_hash>.*),')
        for hash_number in range(number_of_hashes):
            tx_hash_line = lines[len(_GetAccountInfoParser._RE) + number_of_hashes]
            match = tx_hash_re.search(tx_hash_line)
            if match:
                result["tx_hashes"].append(match.group("tx_hash"))
            else:
                raise InvalidArgumentsException("Bad hash string")
        return AccountInfo(result['type'], result['address'], result['balance'], result['nonce'], result['tx_hashes'])


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
    def parse(text: str) -> Transaction:
        result = dict()
        for line in text.split('\n'):
            for key, rx in _GetTransactionParser._RE.items():
                match = rx[0].search(line)
                if match:
                    result[key] = _GetTransactionParser._RE[key][1](match.group(key))

        if len(result) != len(_GetTransactionParser._RE):
            raise InvalidArgumentsException("Not full message")
        return Transaction(result["tx_type"], result["from_address"], result["to_address"], result["value"],
                           result["fee"], result["timestamp"], result["data"], result["signature"])


class _GetBlockParser:
    _RE = {
        'block_hash': (re.compile(r'Block hash (?P<block_hash>.*)'), str),
        'depth': (re.compile(r'Depth: (?P<depth>\d+)'), int),
        'timestamp': (re.compile(r'Timestamp: (?P<timestamp>\d+)'), int),
        'coinbase': (re.compile(r'Coinbase: (?P<coinbase>.*)'), str),
        'previous_block_hash': (re.compile(r'Previous block hash: (?P<previous_block_hash>.*)'), str),
        'number_of_transactions': (re.compile(r'Number of transactions: (?P<number_of_transactions>\d+)'), int)
    }

    @staticmethod
    def parse(text: str) -> Block:
        result = dict()
        lines = text.split('\n')
        header_lines = lines[0:len(_GetBlockParser._RE)]
        for line in header_lines:
            for key, rx in _GetBlockParser._RE.items():
                match = rx[0].search(line)
                if match:
                    result[key] = _GetBlockParser._RE[key][1](match.group(key))

        if len(result) != len(_GetBlockParser._RE):
            raise InvalidArgumentsException("Not full message")

        result["transactions"] = list()
        for tx_number in range(result["number_of_transactions"]):
            tx_lines = lines[len(_GetBlockParser._RE) + 1 + tx_number * len(_GetTransactionParser._RE):len(
                _GetBlockParser._RE) + (tx_number + 1) * len(_GetTransactionParser._RE) + 1]

            result["transactions"].append(_GetTransactionParser.parse("\n".join(tx_lines)))

        return Block(result["block_hash"], result["depth"], result["timestamp"], result["coinbase"],
                     result["previous_block_hash"], result["transactions"])


class _PushContractParser:
    _HASH_TARGET = "transaction_hash"
    _HASH_RE_PARSER = re.compile(r'Created transaction with hash\[hex\]: (?P<transaction_hash>.*)\n')
    _ADDRESS_TARGET = "address"
    _FEE_LEFT_TARGET = "fee_left"
    _RE_PARSER = re.compile(
        r'Remote call of creation smart contract success -> contract created at \[(?P<address>.*)\], fee left\[(?P<fee_left>\d+)\]\n')

    @staticmethod
    def parse(text: str) -> DeployedContract:
        match = _PushContractParser._RE_PARSER.search(text)
        if not match:
            raise InvalidArgumentsException("Not full message")
        address = match.group(_PushContractParser._ADDRESS_TARGET)
        gas_left = int(match.group(_PushContractParser._FEE_LEFT_TARGET))

        match = _PushContractParser._HASH_RE_PARSER.search(text)
        if not match:
            raise InvalidArgumentsException("Not full message")
        tx_hash = match.group(_PushContractParser._HASH_TARGET)
        return DeployedContract(tx_hash, address, gas_left)


class _MessageCallParser:
    _HASH_TARGET = "transaction_hash"
    _HASH_RE_PARSER = re.compile(r'Created transaction with hash\[hex\]: (?P<transaction_hash>.*)\n')
    _MESSAGE_TARGET = "message"
    _FEE_LEFT_TARGET = "fee_left"
    _RE_PARSER = re.compile(
        r'Remote call of smart contract call success -> contract response\[(?P<message>.*)\], fee left\[(?P<fee_left>\d+)\]')

    @staticmethod
    def parse(text: str) -> ContractResult:
        match = _MessageCallParser._RE_PARSER.search(text)
        if not match:
            raise InvalidArgumentsException("Not full message")
        message = match.group(_MessageCallParser._MESSAGE_TARGET)
        fee_left = int(match.group(_MessageCallParser._FEE_LEFT_TARGET))

        match = _MessageCallParser._HASH_RE_PARSER.search(text)
        if not match:
            raise InvalidArgumentsException("Not full message")
        tx_hash = match.group(_PushContractParser._HASH_TARGET)

        return ContractResult(tx_hash, fee_left, message)


class _CompileContractParser:
    @staticmethod
    def parse(text: str) -> list:
        result = list()
        is_valid = False
        for line in text.split('\n'):
            if line == 'Compiled contracts:':
                is_valid = True
                continue
            if is_valid and line:
                result.append(line.strip())
        return result


class _EncodeParser:
    @staticmethod
    def parse(text: str) -> str:
        return text[:len(text) - 1]


class _DecodeParser:
    @staticmethod
    def parse(text: str) -> dict:
        return json.loads(text)


class _CallViewParser:
    _TARGET = "response"
    _RE_PARSER = re.compile(r'View of smart contract response: (?P<response>.*)\n')

    @staticmethod
    def parse(text: str) -> str:
        match = _CallViewParser._RE_PARSER.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid message")
        message = match.group(_CallViewParser._TARGET)
        return message


ZERO_WAIT = 0
MINIMUM_TX_WAIT = 3
MINIMAL_CALL_TIMEOUT = 7
MINIMAL_STANDALONE_TIMEOUT = 5
MINIMAL_TRANSACTION_TIMEOUT = 10
MINIMAL_CONTRACT_TIMEOUT = 15


class Client(BaseClient):
    def __init__(self, *, name: str, client_file_path: str, work_dir: str, node_address: str, is_http: bool,
                 logger: Logger):
        self.name = name
        self.client_file_path = client_file_path
        self.work_dir = work_dir
        self.connect_params = ["--host", node_address]
        if is_http:
            self.connect_params.append("--http")
        self.logger = logger

    def __run_standalone_command(self, *, command, parameters, timeout) -> str:
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

    def __run_client_command(self, *, command, parameters, timeout, wait) -> str:
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

    def connection_test(self, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> bool:
        result = self.__run_client_command(command="connection_test", parameters=[], timeout=timeout, wait=wait)
        return _TestConnectionParser.parse(result)

    def node_info(self, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> NodeInfo:
        result = self.__run_client_command(command="node_info", parameters=[], timeout=timeout, wait=wait)
        return _NodeInfoParser.parse(result)

    def generate_keys(self, *, keys_path: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> Keys:
        path = os.path.abspath(keys_path)
        if not os.path.exists(path):
            os.makedirs(path)
        result = self.__run_standalone_command(command="generate_keys", parameters=["--keys", path], timeout=timeout)
        return _GenerateKeysParser.parse(result)

    def load_address(self, *, keys_path: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> Keys:
        result = self.__run_standalone_command(command="keys_info", parameters=["--keys", keys_path], timeout=timeout)
        return Keys(keys_path, _KeysInfoParser.parse(result))

    def get_balance(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> int:
        parameters = ["--address", address]
        result = self.__run_client_command(command="get_balance", parameters=parameters, timeout=timeout, wait=wait)
        return _GetBalanceParser.parse(result)

    def get_account_info(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT,
                         wait=ZERO_WAIT) -> AccountInfo:  # TODO fix parser for different account types
        parameters = ["--address", address]
        result = self.__run_client_command(command="get_account_info", parameters=parameters, timeout=timeout,
                                           wait=wait)
        return _GetAccountInfoParser.parse(result)

    def transfer(self, *, to_address: str, amount: int, from_address: Keys, fee: int, wait=MINIMUM_TX_WAIT,
                 timeout=MINIMAL_TRANSACTION_TIMEOUT) -> TransferResult:
        parameters = ["--to", to_address, "--amount", str(amount), "--keys", from_address.keys_path, "--fee",
                      str(fee)]
        result = self.__run_client_command(command="transfer", parameters=parameters, timeout=timeout, wait=wait)
        return _TransferParser.parse(result)

    def get_transaction_status(self, *, tx_hash: str, wait: int, timeout: int) -> TransactionStatus:
        raise LogicException("method is not implemented")

    def get_transaction(self, *, tx_hash: str, wait=ZERO_WAIT, timeout=MINIMAL_CALL_TIMEOUT) -> Transaction:
        parameters = ["--hash", tx_hash]
        result = self.__run_client_command(command="get_transaction", parameters=parameters, timeout=timeout, wait=wait)
        return _GetTransactionParser.parse(result)

    def get_block(self, block_hash=None, block_number=None, *, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> Block:
        if block_hash is not None:
            result = self.__run_client_command(command="get_block", parameters=["--hash", block_hash], timeout=timeout,
                                               wait=wait)
        elif block_number is not None:
            result = self.__run_client_command(command="get_block", parameters=["--number", str(block_number)],
                                               timeout=timeout, wait=wait)
        else:
            raise InvalidArgumentsException("block_hash and block_number were not set")
        return _GetBlockParser.parse(result)

    def compile_file(self, *, code: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> list:
        result = self.__run_standalone_command(command="compile_contract", parameters=["--code", code], timeout=timeout)
        return _CompileContractParser.parse(result)

    def encode_message(self, *, code: str, message: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> str:
        parameters = ["--code", code, "--message", message]
        result = self.__run_standalone_command(command="encode_message", parameters=parameters, timeout=timeout)
        return _EncodeParser.parse(result)

    def decode_message(self, *, code: str, method: str, message: str, timeout=MINIMAL_STANDALONE_TIMEOUT) -> dict:
        parameters = ["--code", code, "--method", method, "--message", message]
        result = self.__run_standalone_command(command="decode_message", parameters=parameters, timeout=timeout)
        return _DecodeParser.parse(result)

    def push_contract(self, *, from_address: Keys, code: str, fee: int, amount: int, init_message: str,
                      timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> DeployedContract:
        parameters = ["--keys", from_address.keys_path, "--code", code, "--amount",
                      str(amount), "--fee", str(fee), "--message", init_message]
        result = self.__run_client_command(command="push_contract", parameters=parameters, timeout=timeout, wait=wait)
        return _PushContractParser.parse(result)

    def message_call(self, *, from_address: Keys, to_address: str, fee: int, amount: int, message: str,
                     timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> ContractResult:
        parameters = ["--keys", from_address.keys_path, "--to", to_address,
                      "--amount", str(amount), "--fee", str(fee), "--message", message]
        result = self.__run_client_command(command="call_contract", parameters=parameters, timeout=timeout, wait=wait)
        return _MessageCallParser.parse(result)

    def call_view(self, *, from_address: Keys, to_address: str, message: str, timeout=MINIMAL_CALL_TIMEOUT,
                  wait=ZERO_WAIT) -> str:
        parameters = ["--keys", from_address.keys_path, "--to", to_address, "--message", message]
        result = self.__run_client_command(command="call_view", parameters=parameters, timeout=timeout, wait=wait)
        return _CallViewParser.parse(result)
