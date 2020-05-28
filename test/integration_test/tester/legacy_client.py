import json
import os
import re
import subprocess
import time

from .base import Logger, InvalidArgumentsException, TimeOutException, LogicException
from .client import BaseClient, NodeInfo, Keys, AccountType, AccountInfo, TransactionStatusCode, TransactionType, \
    TransactionStatus, Transaction, Block


class _TestConnectionParser:
    @staticmethod
    def parse(text: str) -> bool:
        return text == "Connection test passed\n"


class _NodeInfoParser:
    __node_info_re = re.compile(r'Top block hash: (?P<top_block_hash>.*)\nTop block number: (?P<top_block_number>\d+)')

    @staticmethod
    def parse(text: str) -> NodeInfo:
        match = _NodeInfoParser.__node_info_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        top_block_hash = match.group('top_block_hash')
        top_block_number = int(match.group('top_block_number'))
        return NodeInfo(top_block_hash, top_block_number)


class _GenerateKeysParser:
    __keys_generate_re = re.compile(r'Generated key at "(?P<keys_path>.*)"\nAddress: (?P<address>.*)')

    @staticmethod
    def parse(text: str) -> Keys:
        match = _GenerateKeysParser.__keys_generate_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        keys_path = match.group('keys_path')
        address = match.group('address')
        return Keys(keys_path, address)


class _KeysInfoParser:
    __keys_info_re = re.compile(r'Address: (?P<address>.*)')

    @staticmethod
    def parse(text: str) -> str:
        match = _KeysInfoParser.__keys_info_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        return match.group("address")


class _GetBalanceParser:
    __balance_re = re.compile(r'Balance of (?P<address>.*) is (?P<balance>\d+)')

    @staticmethod
    def parse(text: str) -> int:
        match = _GetBalanceParser.__balance_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        return int(match.group("balance"))


class _GetAccountInfoParser:
    __account_info_re = re.compile(
        r'(?P<type>.*) address: (?P<address>.*)\n\t+Balance: (?P<balance>.*)\n\t+Nonce: (?P<nonce>\d+)\n\t+')
    __tx_hash_re = re.compile(r'(?P<tx_hash>.*)\n')

    @staticmethod
    def parse(text: str) -> AccountInfo:
        match = _GetAccountInfoParser.__account_info_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        account_type = AccountType(match.group('type'))
        address = match.group('address')
        balance = int(match.group('balance'))
        if account_type == AccountType.CLIENT:
            nonce = int(match.group('nonce'))
            tx_hashes = list()
            for hash_line in text.split('\n')[4:-2]:
                match = _GetAccountInfoParser.__tx_hash_re.search(hash_line)
                if match:
                    tx_hashes.append(match.group("tx_hash"))
                else:
                    raise InvalidArgumentsException("Bad hash string")
            return AccountInfo(account_type, address, balance, nonce, tx_hashes)
        return AccountInfo(account_type, address, balance, 0, list())


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


class _GetTransactionStatusParser:
    __transaction_status_re = re.compile(
        r'Type: (?P<type>.*)\n\t+Status: (?P<status>.*|)\n\t+Fee left: (?P<fee_left>.*)\n')
    __tx_hash_re = re.compile(r'Transaction with hash\[hex\]: (?P<tx_hash>.*)\n')
    __message_with_address_re = re.compile(r'\t+Message: new contract address (?P<message>.*)\n')
    __message_re = re.compile(r'\t+Message: (?P<message>.*)\n')

    @staticmethod
    def parse(text: str) -> TransactionStatus:
        match = _GetTransactionStatusParser.__tx_hash_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        tx_hash = match.group('tx_hash')
        match = _GetTransactionStatusParser.__transaction_status_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        action_type = TransactionType(match.group('type'))
        status_type = TransactionStatusCode(match.group('status'))
        fee_left = int(match.group('fee_left'))
        if action_type == TransactionType.CONTRACT_CREATION:
            match = _GetTransactionStatusParser.__message_with_address_re.search(text)
            if not match:
                raise InvalidArgumentsException("Invalid command output")
            data = match.group("message")
            return TransactionStatus(action_type, status_type, tx_hash, fee_left, data)

        match = _GetTransactionStatusParser.__message_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        data = match.group("message")
        return TransactionStatus(action_type, status_type, tx_hash, fee_left, data)


class _GetTransactionParser:
    __get_transaction_re = re.compile(
        r'Type: (?P<tx_type>.*)\n\t+From: (?P<from_address>.*)\n\t+To: (?P<to_address>.*)\n\t+Value: (?P<value>.*)\n\t+Fee: (?P<fee>.*)\n\t+Timestamp: (?P<timestamp>\d+)\n\t+Data: (?P<data>.*)\n\t+Signature: (?P<signature>.*)')

    @staticmethod
    def parse(text: str) -> Transaction:
        match = _GetTransactionParser.__get_transaction_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        tx_type = TransactionType(match.group("tx_type"))
        from_address = match.group("from_address")
        to_address = match.group("to_address")
        value = int(match.group("value"))
        fee = int(match.group("fee"))
        timestamp = int(match.group("timestamp"))
        signature = match.group("signature")
        if signature == "verified":
            signature_status = True
        else:
            signature_status = False
        data = match.group("data")
        if data == "<empty>":
            data = ""

        return Transaction(tx_type, from_address, to_address, value, fee, timestamp, data, signature_status)


class _GetBlockParser:
    __get_block_re = re.compile(
        r'\n\t+Depth: (?P<depth>\d+)\n\t+Timestamp: (?P<timestamp>\d+)\n\t+Coinbase: (?P<coinbase>.*)\n\t+Nonce: (?P<nonce>\d+)\n\t+Previous block hash: (?P<previous_block_hash>.*)\n\t+Number of transactions: (?P<number_of_transactions>\d+)\n')

    @staticmethod
    def parse(text: str) -> Block:
        match = _GetBlockParser.__get_block_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid command output")
        depth = int(match.group("depth"))
        timestamp = int(match.group("timestamp"))
        nonce = int(match.group("nonce"))
        coinbase = match.group("coinbase")
        previous_block_hash = match.group("previous_block_hash")
        number_of_transactions = int(match.group("number_of_transactions"))

        transactions = list()
        lines = text.split('\n')
        for tx_number in range(number_of_transactions):
            tx_lines = lines[7 + 1 + tx_number * 8:7 + (tx_number + 1) * 8 + 1]
            transactions.append(_GetTransactionParser.parse("\n".join(tx_lines)))

        return Block(depth, nonce, timestamp, coinbase, previous_block_hash, transactions)


class _CallViewParser:
    __call_view_re = re.compile(r'View of smart contract response: (?P<response>.*)\n')

    @staticmethod
    def parse(text: str) -> str:
        match = _CallViewParser.__call_view_re.search(text)
        if not match:
            raise InvalidArgumentsException("Invalid message")
        message = match.group('response')
        return message


ZERO_WAIT = 0
MINIMUM_TX_WAIT = 4
MINIMAL_CALL_TIMEOUT = 20
MINIMAL_STANDALONE_TIMEOUT = 10
MINIMAL_TRANSACTION_TIMEOUT = 20
MINIMAL_CONTRACT_TIMEOUT = 20


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

    def get_account_info(self, *, address: str, timeout=MINIMAL_CALL_TIMEOUT, wait=ZERO_WAIT) -> AccountInfo:
        parameters = ["--address", address]
        result = self.__run_client_command(command="get_account_info", parameters=parameters, timeout=timeout,
                                           wait=wait)
        return _GetAccountInfoParser.parse(result)

    def transfer(self, *, to_address: str, amount: int, from_address: Keys, fee: int, wait=MINIMUM_TX_WAIT,
                 timeout=MINIMAL_TRANSACTION_TIMEOUT) -> TransactionStatus:
        parameters = ["--to", to_address, "--amount", str(amount), "--keys", from_address.keys_path, "--fee",
                      str(fee)]
        result = self.__run_client_command(command="transfer", parameters=parameters, timeout=timeout, wait=wait)
        return _GetTransactionStatusParser.parse(result)

    def get_transaction_status(self, *, tx_hash: str, wait=ZERO_WAIT, timeout=MINIMAL_CALL_TIMEOUT) -> TransactionStatus:
        parameters = ["--hash", tx_hash]
        result = self.__run_client_command(command="get_transaction_status", parameters=parameters, timeout=timeout,
                                           wait=wait)
        return _GetTransactionStatusParser.parse(result)

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
                      timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> TransactionStatus:
        parameters = ["--keys", from_address.keys_path, "--code", code, "--amount",
                      str(amount), "--fee", str(fee), "--message", init_message]
        result = self.__run_client_command(command="push_contract", parameters=parameters, timeout=timeout, wait=wait)
        return _GetTransactionStatusParser.parse(result)

    def message_call(self, *, from_address: Keys, to_address: str, fee: int, amount: int, message: str,
                     timeout=MINIMAL_CONTRACT_TIMEOUT, wait=MINIMUM_TX_WAIT) -> TransactionStatus:
        parameters = ["--keys", from_address.keys_path, "--to", to_address,
                      "--amount", str(amount), "--fee", str(fee), "--message", message]
        result = self.__run_client_command(command="call_contract", parameters=parameters, timeout=timeout, wait=wait)
        return _GetTransactionStatusParser.parse(result)

    def call_view(self, *, from_address: Keys, to_address: str, message: str, timeout=MINIMAL_CALL_TIMEOUT,
                  wait=ZERO_WAIT) -> str:
        parameters = ["--keys", from_address.keys_path, "--to", to_address, "--message", message]
        result = self.__run_client_command(command="call_view", parameters=parameters, timeout=timeout, wait=wait)
        return _CallViewParser.parse(result)
