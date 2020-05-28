import enum
from abc import ABCMeta

from .base import LogicException


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


class BaseClient(metaclass=ABCMeta):
    def connection_test(self, *, timeout, wait) -> bool:
        raise LogicException("method is not implemented")

    def node_info(self, *, timeout: int, wait: int) -> NodeInfo:
        raise LogicException("method is not implemented")

    def generate_keys(self, *, keys_path: str, timeout: int) -> Keys:
        raise LogicException("method is not implemented")

    def load_address(self, *, keys_path: str, timeout: int) -> Keys:
        raise LogicException("method is not implemented")

    def get_balance(self, *, address: str, timeout: int, wait: int) -> int:
        raise LogicException("method is not implemented")

    def get_account_info(self, *, address: str, timeout: int, wait: int) -> AccountInfo:
        raise LogicException("method is not implemented")

    def transfer(self, *, to_address: str, amount: int, from_address: Keys, fee: int, wait: int,
                 timeout: int) -> TransactionStatus:
        raise LogicException("method is not implemented")

    def get_transaction_status(self, *, tx_hash: str, wait: int, timeout: int) -> TransactionStatus:
        raise LogicException("method is not implemented")

    def get_transaction(self, *, tx_hash: str, wait: int, timeout: int) -> Transaction:
        raise LogicException("method is not implemented")

    def get_block(self, block_hash=None, block_number=None, *, timeout: int, wait: int) -> Block:
        raise LogicException("method is not implemented")

    def compile_file(self, *, code: str, timeout: int) -> list:
        raise LogicException("method is not implemented")

    def encode_message(self, *, code: str, message: str, timeout: int) -> str:
        raise LogicException("method is not implemented")

    def decode_message(self, *, code: str, method: str, message: str, timeout: int) -> dict:
        raise LogicException("method is not implemented")

    def push_contract(self, *, from_address: Keys, code: str, fee: int, amount: int, init_message: str, timeout: int,
                      wait: int) -> TransactionStatus:
        raise LogicException("method is not implemented")

    def message_call(self, *, from_address: Keys, to_address: str, fee: int, amount: int, message: str, timeout: int,
                     wait: int) -> TransactionStatus:
        raise LogicException("method is not implemented")

    def call_view(self, *, from_address: Keys, to_address: str, message: str, timeout: int, wait: int) -> str:
        raise LogicException("method is not implemented")
