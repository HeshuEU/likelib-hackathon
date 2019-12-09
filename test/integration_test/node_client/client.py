import logging

import grpc

import public_rpc_pb2
import public_rpc_pb2_grpc


def get_balance(stub):
    fake_address = public_rpc_pb2.Address(address="fake address")
    balance_response = stub.balance(fake_address)
    print("balance request: money[", balance_response.money, "]")


def make_transfer(stub):
    money = public_rpc_pb2.Money(money=1)
    from_address = public_rpc_pb2.Address(address="fake from address")
    to_address = public_rpc_pb2.Address(address="fake to address")
    transaction = public_rpc_pb2.Transaction(amount=money, from_address=from_address, to_address=to_address)
    transaction_response = stub.transaction(transaction)
    print("transaction request: hash[", transaction_response.hash_string, "]")


def run():
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = public_rpc_pb2_grpc.NodeStub(channel)
        get_balance(stub)
        make_transfer(stub)


if __name__ == '__main__':
    logging.basicConfig()
    run()
